#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "context.h"
#include "mathf/mathf.h"
#include "mathf/vectors.h"
#include "mathf/matries.h"
#include "mathf/quat.h"

const int MAX_FRAMES_IN_FLIGHT = 3;

typedef struct {
    vec3 pos;
    vec3 normal;
    vec2 texCoord;
} Vertex;

typedef struct {
    mat4 model;
    mat4 view;
    mat4 proj;
} UniformBufferObject;

typedef struct {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    uint32_t indexCount;
    uint32_t vertexCount;
} Mesh;

typedef struct
{
    VkSwapchainKHR swapchain;
    VkRenderPass renderPass;

    VkFormat format;
    VkExtent2D extent;

    uint32_t imageCount;

    VkImage *images;
    VkImageView *views;
    VkFramebuffer *framebuffers;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    VkCommandPool commandPool;
    VkCommandBuffer *commandBuffers;

    VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];
    VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSets[MAX_FRAMES_IN_FLIGHT];

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<Mesh> meshes;

    VkBuffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
    VkDeviceMemory uniformBuffersMemory[MAX_FRAMES_IN_FLIGHT];
    void* uniformBuffersMapped[MAX_FRAMES_IN_FLIGHT];

    uint32_t currentFrame = 0;

    
} Renderer;

void create_swapchain(Context *ctx, Renderer *ren)
{
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        ctx->physicalDevice,
        ctx->surface,
        &caps);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        ctx->physicalDevice,
        ctx->surface,
        &formatCount,
        nullptr);

    if (formatCount == 0) {
        fprintf(stderr, "failed to find surface formats!\n");
        exit(EXIT_FAILURE);
    }

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        ctx->physicalDevice,
        ctx->surface,
        &formatCount,
        formats.data());

    VkSurfaceFormatKHR chosen = formats[0];
    for (const auto& availableFormat : formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            chosen = availableFormat;
            break;
        }
    }

    VkSwapchainCreateInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

    info.surface = ctx->surface;
    info.minImageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && info.minImageCount > caps.maxImageCount) {
        info.minImageCount = caps.maxImageCount;
    }

    info.imageFormat = chosen.format;
    info.imageColorSpace = chosen.colorSpace;

    info.imageExtent = caps.currentExtent;

    info.imageArrayLayers = 1;

    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    info.preTransform = caps.currentTransform;

    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    info.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    info.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(ctx->device, &info, nullptr, &ren->swapchain) != VK_SUCCESS) {
        fprintf(stderr, "failed to create swap chain!\n");
        exit(EXIT_FAILURE);
    }

    ren->format = chosen.format;
    ren->extent = caps.currentExtent;

    vkGetSwapchainImagesKHR(
        ctx->device,
        ren->swapchain,
        &ren->imageCount,
        nullptr);

    ren->images = (VkImage *)malloc(sizeof(VkImage) * ren->imageCount);

    vkGetSwapchainImagesKHR(
        ctx->device,
        ren->swapchain,
        &ren->imageCount,
        ren->images);

    ren->views = (VkImageView *)malloc(sizeof(VkImageView) * ren->imageCount);

    for (uint32_t i = 0; i < ren->imageCount; i++)
    {
        VkImageViewCreateInfo view{};
        view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

        view.image = ren->images[i];

        view.viewType = VK_IMAGE_VIEW_TYPE_2D;

        view.format = ren->format;

        view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view.subresourceRange.levelCount = 1;
        view.subresourceRange.layerCount = 1;

        if (vkCreateImageView(ctx->device, &view, nullptr, &ren->views[i]) != VK_SUCCESS) {
            fprintf(stderr, "failed to create image views!\n");
            exit(EXIT_FAILURE);
        }
    }
}
