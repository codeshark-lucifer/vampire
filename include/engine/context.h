#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdio>
#include <cstring>
#include <GLFW/glfw3.h>
#include "platform.h"

typedef struct
{
    VkAllocationCallbacks *allocator = nullptr;

    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkQueue queue;
    uint32_t queueFamily;

    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    bool enableValidationLayers = true;
} Context;

const char *validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"};

bool check_validation_layer_support()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    fprintf(stderr, "Validation Layer: %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

VkResult create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debug_callback;
}

void create_instance(Config *cfg, Context *ctx)
{
    if (ctx->enableValidationLayers && !check_validation_layer_support())
    {
        fprintf(stderr, "Validation layers requested, but not available! Disabling them.\n");
        ctx->enableValidationLayers = false;
    }

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (ctx->enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = cfg->application_name;
    appInfo.pEngineName = cfg->engine_name;
    appInfo.apiVersion = cfg->api_version;

    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &appInfo;
    info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    info.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (ctx->enableValidationLayers)
    {
        info.enabledLayerCount = 1;
        info.ppEnabledLayerNames = validationLayers;

        populate_debug_messenger_create_info(debugCreateInfo);
        info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    }
    else
    {
        info.enabledLayerCount = 0;
        info.pNext = nullptr;
    }

    if (vkCreateInstance(&info, ctx->allocator, &ctx->instance) != VK_SUCCESS)
    {
        fprintf(stderr, "failed to create instance!\n");
        exit(EXIT_FAILURE);
    }

    if (ctx->enableValidationLayers)
    {
        if (create_debug_utils_messenger_ext(ctx->instance, &debugCreateInfo, ctx->allocator, &ctx->debugMessenger) != VK_SUCCESS)
        {
            fprintf(stderr, "failed to set up debug messenger!\n");
        }
    }
}

void select_physical_device(Context *ctx)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(ctx->instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        fprintf(stderr, "failed to find GPUs with Vulkan support!\n");
        exit(EXIT_FAILURE);
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(ctx->instance, &deviceCount, devices.data());

    for (auto &d : devices)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(d, &props);

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            ctx->physicalDevice = d;
            return;
        }
    }

    ctx->physicalDevice = devices[0];
}

void select_queue_family(Context *ctx)
{
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &count, nullptr);

    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &count, families.data());

    for (uint32_t i = 0; i < count; i++)
    {
        VkBool32 presentSupport = false;

        vkGetPhysicalDeviceSurfaceSupportKHR(
            ctx->physicalDevice,
            i,
            ctx->surface,
            &presentSupport);

        if ((families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport)
        {
            ctx->queueFamily = i;
            break;
        }
    }
}

void create_logical_device(Context *ctx)
{
    float priority = 1.0f;

    const char *extensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceQueueCreateInfo queue{};
    queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue.queueFamilyIndex = ctx->queueFamily;
    queue.queueCount = 1;
    queue.pQueuePriorities = &priority;

    VkDeviceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.queueCreateInfoCount = 1;
    info.pQueueCreateInfos = &queue;

    info.enabledExtensionCount = 1;
    info.ppEnabledExtensionNames = extensions;

    if (ctx->enableValidationLayers)
    {
        info.enabledLayerCount = 1;
        info.ppEnabledLayerNames = validationLayers;
    }
    else
    {
        info.enabledLayerCount = 0;
    }

    if (vkCreateDevice(ctx->physicalDevice, &info, ctx->allocator, &ctx->device) != VK_SUCCESS)
    {
        fprintf(stderr, "failed to create logical device!\n");
        exit(EXIT_FAILURE);
    }

    vkGetDeviceQueue(ctx->device, ctx->queueFamily, 0, &ctx->queue);
}

uint32_t find_memory_type(Context *ctx, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(ctx->physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    fprintf(stderr, "failed to find suitable memory type!\n");
    exit(EXIT_FAILURE);
}

VkFormat find_supported_format(Context *ctx, const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(ctx->physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    fprintf(stderr, "failed to find supported format!\n");
    exit(EXIT_FAILURE);
}
