#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#if defined(_WIN32) || defined(_WIN64)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")
#endif

#include <engine/utils.h>
#include <vulkan/vulkan.h>

void callback_key(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS))
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void callback_erorr(int code, const char *description)
{
    PANIC(code, "GLFW: %s", description);
    exit(EXIT_FAILURE);
}

void callback_exit()
{
    printf("Exit Engine.");
    glfwTerminate();
}

struct State
{
    const char *application_name;
    const char *engine_name;

    const char *window_title;
    int window_width, window_height;
    bool window_resizeable;
    bool window_fullscreen;

    u32 api_version;
    u32 queueFamily;

    GLFWmonitor *window_monitor;
    GLFWwindow *window;

    VkAllocationCallbacks *allocator;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    VkDevice device;
    VkQueue queue;

    VkSwapchainKHR swapchain;
    VkFormat swapchainFormat;
    VkExtent2D swapchainExtent;

    uint32_t imageCount;
    VkImage* swapchainImages;
    VkImageView* swapchainImageViews;
};

void create_window(State *state)
{
    PANIC(!glfwInit(), "Failed to Initialize GLFW.");
    // 1. Set hints
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, (*state).window_resizeable ? GLFW_TRUE : GLFW_FALSE);
    // We use this (*state). is same as (*state).

    if ((*state).window_fullscreen)
    {
        (*state).window_monitor = glfwGetPrimaryMonitor();

        const GLFWvidmode *mode = glfwGetVideoMode((*state).window_monitor);
        (*state).window_width = mode->width;
        (*state).window_height = mode->height;
    }
    // 2. CREATE THE WINDOW FIRST
    (*state).window = glfwCreateWindow(
        (*state).window_width, (*state).window_height,
        (*state).window_title,
        (*state).window_monitor,
        NULL);

    if (!(*state).window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // 3. NOW apply the Windows-specific attributes
#if defined(_WIN32) || defined(_WIN64)
    HWND hwnd = glfwGetWin32Window((*state).window);

    BOOL useDarkMode = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));

    COLORREF titleBarColor = RGB(30, 30, 30); // Dark Grey
    DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &titleBarColor, sizeof(titleBarColor));

    COLORREF textColor = RGB(255, 255, 255); // White
    DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &textColor, sizeof(textColor));
#endif
}

void create_instance(State *state)
{
    u32 extension_counter;
    const char **extensions = glfwGetRequiredInstanceExtensions(&extension_counter);

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = (*state).application_name,
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = (*state).engine_name,
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = (*state).api_version};

    VkInstanceCreateInfo createinfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = extension_counter,
        .ppEnabledExtensionNames = extensions};

    PANIC(vkCreateInstance(&createinfo, (*state).allocator, &(*state).instance), "Failed to create instance.");
}

void create_logger(State *state)
{
    u32 api_version, variant, major, minor, patch;
    PANIC(vkEnumerateInstanceVersion(&api_version), "Failed to load enumerate instance version.");

    variant = VK_API_VERSION_VARIANT(api_version);
    major = VK_API_VERSION_MAJOR(api_version);
    minor = VK_API_VERSION_MINOR(api_version);
    patch = VK_API_VERSION_PATCH(api_version);

    printf("*Start Engine:\n\t- Vulkan API %i.%i.%i.%i\n", variant, major, minor, patch);
    printf("\t- GLFW %s\n", glfwGetVersionString());
}

void select_physical_device(State *state)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(state->instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        PANIC(true, "Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(state->instance, &deviceCount, devices.data());

    // Instead of just taking the first one, we usually "score" them
    for (const auto &device : devices)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        // Example: Favor Discrete GPUs over Integrated ones
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            state->physicalDevice = device;
            return;
        }
    }

    // Fallback: Just take the first one if no discrete GPU was found
    state->physicalDevice = devices[0];
}

void create_surface(State *state)
{
    PANIC(glfwCreateWindowSurface(state->instance, state->window, state->allocator, &state->surface), "Couldn't create window surface.");
}

void select_queue_family(State *state)
{
    state->queueFamily = UINT32_MAX;
    u32 count;

    vkGetPhysicalDeviceQueueFamilyProperties(state->physicalDevice, &count, NULL);

    array<VkQueueFamilyProperties> queueFamilies(count);
    vkGetPhysicalDeviceQueueFamilyProperties(state->physicalDevice, &count, queueFamilies.data());

    for (int queueIdx = 0; queueIdx < count; ++queueIdx)
    {
        VkQueueFamilyProperties properties = queueFamilies[queueIdx];
        if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT && glfwGetPhysicalDevicePresentationSupport(state->instance, state->physicalDevice, queueIdx))
        {
            state->queueFamily = queueIdx;
            break;
        }
    }

    PANIC(state->queueFamily == UINT32_MAX, "Could'nt find suitable queue family.");
}

void create_device(State *state)
{
    // Define these as actual variables so we can take their addresses safely
    float queuePriority = 1.0f;
    const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = state->queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority // Safe now
    };

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, // Note: Fix this sType! (See below)
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = deviceExtensions, // Safe now
        .pEnabledFeatures = nullptr,
    };

    PANIC(vkCreateDevice(state->physicalDevice, &createInfo, state->allocator, &state->device), "Couldn't create device.");
}

void get_queue(State *state)
{
    u32 queueFamilyIndex = 0;

    vkGetDeviceQueue(state->device, queueFamilyIndex, 0, &state->queue);
}

void create_swapchain(State *state)
{
    // 1. Query basic surface capabilities
    VkSurfaceCapabilitiesKHR capabilities;
    PANIC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(state->physicalDevice, state->surface, &capabilities),
          "Couldn't get capabilities.");

    // 2. Choose the best surface format
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(state->physicalDevice, state->surface, &formatCount, NULL);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(state->physicalDevice, state->surface, &formatCount, formats.data());

    VkSurfaceFormatKHR selectedFormat = formats[0];
    for (const auto& f : formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            selectedFormat = f;
            break;
        }
    }

    // 3. Choose Present Mode
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(state->physicalDevice, state->surface, &presentModeCount, NULL);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(state->physicalDevice, state->surface, &presentModeCount, presentModes.data());

    VkPresentModeKHR selectedMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& m : presentModes) {
        if (m == VK_PRESENT_MODE_MAILBOX_KHR) {
            selectedMode = m;
            break;
        }
    }

    // 4. Determine Image Count
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    // 5. Build the Create Info
    VkSwapchainKHR newSwapchain;
    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = state->surface,
        .minImageCount = imageCount,
        .imageFormat = selectedFormat.format,
        .imageColorSpace = selectedFormat.colorSpace,
        .imageExtent = capabilities.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = selectedMode,
        .clipped = VK_TRUE,
        .oldSwapchain = state->swapchain
    };

    PANIC(vkCreateSwapchainKHR(state->device, &createInfo, state->allocator, &newSwapchain), 
          "Failed to create swapchain!");

    // Destroy old swapchain if recreating
    if (state->swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(state->device, state->swapchain, state->allocator);
    }

    state->swapchain = newSwapchain;
    state->swapchainFormat = selectedFormat.format;
    state->swapchainExtent = capabilities.currentExtent;

    // 6. Retrieve Images
    vkGetSwapchainImagesKHR(state->device, state->swapchain, &state->imageCount, NULL);
    state->swapchainImages = (VkImage*)malloc(sizeof(VkImage) * state->imageCount);
    vkGetSwapchainImagesKHR(state->device, state->swapchain, &state->imageCount, state->swapchainImages);

    // 7. Create Image Views
    
    state->swapchainImageViews = (VkImageView*)malloc(sizeof(VkImageView) * state->imageCount);
    for (uint32_t i = 0; i < state->imageCount; i++) {
        VkImageViewCreateInfo viewInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = state->swapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = state->swapchainFormat,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0, .levelCount = 1,
                .baseArrayLayer = 0, .layerCount = 1
            }
        };
        PANIC(vkCreateImageView(state->device, &viewInfo, state->allocator, &state->swapchainImageViews[i]), 
              "Failed to create image view!");
    }
}

void init(State *state)
{
    glfwSetErrorCallback(callback_erorr);
    atexit(callback_exit);

    create_window(state);
    create_instance(state);
    create_logger(state);

    select_physical_device(state);
    create_surface(state);
    select_queue_family(state);
    create_device(state);

    get_queue(state);

    create_swapchain(state);

    glfwSetKeyCallback((*state).window, callback_key);
}

void loop(State *state)
{
    while (!glfwWindowShouldClose((*state).window))
    {
        glfwPollEvents();
    }
}

void cleanup(State *state)
{
    // Wait for the GPU to finish before destroying things
    if (state->device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(state->device);
    }

    // 1. Destroy Image Views (must happen before swapchain)
    if (state->swapchainImageViews) {
        for (uint32_t i = 0; i < state->imageCount; i++) {
            vkDestroyImageView(state->device, state->swapchainImageViews[i], state->allocator);
        }
        free(state->swapchainImageViews);
        state->swapchainImageViews = nullptr;
    }

    // 2. Destroy Swapchain
    if (state->swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(state->device, state->swapchain, state->allocator);
        state->swapchain = VK_NULL_HANDLE;
    }

    // 3. Free the images handle array (the images themselves are destroyed by the swapchain)
    if (state->swapchainImages) {
        free(state->swapchainImages);
        state->swapchainImages = nullptr;
    }

    // 4. Standard cleanup
    vkDestroyDevice(state->device, state->allocator);
    vkDestroySurfaceKHR(state->instance, state->surface, state->allocator);
    
    if (state->window) {
        glfwDestroyWindow(state->window);
    }

    vkDestroyInstance(state->instance, state->allocator);
}

int main()
{
    State state = {
        .application_name = "Vampire",
        .engine_name = "AtlasEngine",

        .window_title = "vamprie",
        .window_width = 956,
        .window_height = 540,
        .window_resizeable = false,
        .window_fullscreen = false,

        .api_version = VK_API_VERSION_1_4};

    init(&state);
    loop(&state);
    cleanup(&state);

    return EXIT_SUCCESS;
}