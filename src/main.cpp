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
    const char *window_title;
    int width, height;
    bool window_resizeable;
    bool window_fullscreen;
    u32 api_version;

    GLFWmonitor *window_monitor;
    GLFWwindow *window;

    VkAllocationCallbacks *allocator;
    VkInstance instance;
};

void create_window(State *state)
{
    PANIC(!glfwInit(), "Failed to Initialize GLFW.");
    // 1. Set hints
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, state->window_resizeable ? GLFW_TRUE : GLFW_FALSE);
    // We use this (*state). is same as state->

    (*state).window_monitor = state->window_fullscreen ? glfwGetPrimaryMonitor() : NULL;

    // 2. CREATE THE WINDOW FIRST
    state->window = glfwCreateWindow(
        (*state).width, (*state).height,
        (*state).window_title,
        (*state).window_monitor,
        NULL);

    if (!state->window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    // 3. NOW apply the Windows-specific attributes
#if defined(_WIN32) || defined(_WIN64)
    HWND hwnd = glfwGetWin32Window(state->window);

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
        .pApplicationName = state->window_title,
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "AtlasEngine",
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = state->api_version};

    VkInstanceCreateInfo createinfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = extension_counter,
        .ppEnabledExtensionNames = extensions};

    PANIC(vkCreateInstance(&createinfo, state->allocator, &state->instance), "Failed to create instance.");
}

void create_logger(State *state)
{
    u32 api_version, variant, major, minor, patch;
    vkEnumerateInstanceVersion(&api_version);
    variant = VK_API_VERSION_VARIANT(api_version);
    major = VK_API_VERSION_MAJOR(api_version);
    minor = VK_API_VERSION_MINOR(api_version);
    patch = VK_API_VERSION_PATCH(api_version);

    printf("Vulkan API %i.%i.%i.%i\n", variant, major, minor, patch);
    printf("GLFW %s\n", glfwGetVersionString());
}

void init(State *state)
{
    glfwSetErrorCallback(callback_erorr);
    atexit(callback_exit);

    create_window(state);
    create_instance(state);
    create_logger(state);

    glfwSetKeyCallback(state->window, callback_key);
}

void loop(State *state)
{
    while (!glfwWindowShouldClose(state->window))
    {
        glfwPollEvents();
    }
}

void cleanup(State *state)
{
    glfwDestroyWindow((*state).window);
    (*state).window = nullptr;
}

int main()
{
    State state = {
        .window_title = "vamprie",
        .width = 956,
        .height = 540,
        .window_resizeable = false,
        .window_fullscreen = false,
        .api_version = VK_API_VERSION_1_4};

    init(&state);
    loop(&state);
    cleanup(&state);

    return EXIT_SUCCESS;
}