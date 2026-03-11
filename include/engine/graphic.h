#pragma once
#include "renderer.h"
#include "utils.h"
#include <fstream>
#include <sstream>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "../../vendor/stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <ecs/ecs.h>

struct TransformComponent
{
    vec3 position;
    vec3 rotation;
    vec3 scale;
};

struct MeshComponent
{
    uint32_t meshIndex; // Index into state->renderer.meshes
};

typedef struct
{
    Config config;
    Window window;
    Context context;
    Renderer renderer;
    ecs::World world;
    bool framebufferResized = false;
} State;

// --- Forward Declarations ---
void create_render_pass(State *state);
void create_framebuffers(State *state);
void create_depth_resources(State *state);
void recreate_swapchain(State *state);

// --- Utilities ---

VkFormat find_depth_format(Context *ctx)
{
    return find_supported_format(
        ctx,
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void create_image(Context *ctx, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    EXPECT(vkCreateImage(ctx->device, &imageInfo, ctx->allocator, &image), "failed to create image!");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(ctx->device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(ctx, memRequirements.memoryTypeBits, properties);

    EXPECT(vkAllocateMemory(ctx->device, &allocInfo, ctx->allocator, &imageMemory), "failed to allocate image memory!");

    vkBindImageMemory(ctx->device, image, imageMemory, 0);
}

VkImageView create_image_view(Context *ctx, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    EXPECT(vkCreateImageView(ctx->device, &viewInfo, ctx->allocator, &imageView), "failed to create image view!");

    return imageView;
}

void cleanup_swapchain(State *state)
{
    if (state->renderer.depthImageView != VK_NULL_HANDLE)
        vkDestroyImageView(state->context.device, state->renderer.depthImageView, state->context.allocator);
    if (state->renderer.depthImage != VK_NULL_HANDLE)
        vkDestroyImage(state->context.device, state->renderer.depthImage, state->context.allocator);
    if (state->renderer.depthImageMemory != VK_NULL_HANDLE)
        vkFreeMemory(state->context.device, state->renderer.depthImageMemory, state->context.allocator);

    for (uint32_t i = 0; i < state->renderer.imageCount; i++)
    {
        if (state->renderer.framebuffers && state->renderer.framebuffers[i] != VK_NULL_HANDLE)
            vkDestroyFramebuffer(state->context.device, state->renderer.framebuffers[i], state->context.allocator);
    }
    if (state->renderer.framebuffers) free(state->renderer.framebuffers);
    state->renderer.framebuffers = nullptr;

    for (uint32_t i = 0; i < state->renderer.imageCount; i++)
    {
        if (state->renderer.views && state->renderer.views[i] != VK_NULL_HANDLE)
            vkDestroyImageView(state->context.device, state->renderer.views[i], state->context.allocator);
    }
    if (state->renderer.views) free(state->renderer.views);
    state->renderer.views = nullptr;
    if (state->renderer.images) free(state->renderer.images);
    state->renderer.images = nullptr;

    vkDestroySwapchainKHR(state->context.device, state->renderer.swapchain, state->context.allocator);
}

void recreate_swapchain(State *state)
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(state->window.handle, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwGetFramebufferSize(state->window.handle, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(state->context.device);

    cleanup_swapchain(state);

    create_swapchain(&state->context, &state->renderer);
    create_depth_resources(state);
    create_framebuffers(state);
}

void create_depth_resources(State *state)
{
    VkFormat depthFormat = find_depth_format(&state->context);

    create_image(&state->context, state->renderer.extent.width, state->renderer.extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, state->renderer.depthImage, state->renderer.depthImageMemory);
    state->renderer.depthImageView = create_image_view(&state->context, state->renderer.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void create_render_pass(State *state)
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = state->renderer.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = find_depth_format(&state->context);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::vector<VkAttachmentDescription> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    EXPECT(vkCreateRenderPass(state->context.device, &renderPassInfo, state->context.allocator, &state->renderer.renderPass), "Failed to create render pass!");
}

void create_framebuffers(State *state)
{
    state->renderer.framebuffers = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * state->renderer.imageCount);

    for (uint32_t i = 0; i < state->renderer.imageCount; i++)
    {
        VkImageView attachments[] = {
            state->renderer.views[i],
            state->renderer.depthImageView};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = state->renderer.renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = state->renderer.extent.width;
        framebufferInfo.height = state->renderer.extent.height;
        framebufferInfo.layers = 1;

        EXPECT(vkCreateFramebuffer(state->context.device, &framebufferInfo, state->context.allocator, &state->renderer.framebuffers[i]), "Failed to create framebuffer!");
    }
}

VkCommandBuffer begin_single_time_commands(State *state)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = state->renderer.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(state->context.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void end_single_time_commands(State *state, VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(state->context.queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(state->context.queue);

    vkFreeCommandBuffers(state->context.device, state->renderer.commandPool, 1, &commandBuffer);
}

void transition_image_layout(State *state, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = begin_single_time_commands(state);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        EXPECT(true, "unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    end_single_time_commands(state, commandBuffer);
}

void copy_buffer_to_image(State *state, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = begin_single_time_commands(state);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1};

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);

    end_single_time_commands(state, commandBuffer);
}

void copy_buffer(State *state, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = begin_single_time_commands(state);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    end_single_time_commands(state, commandBuffer);
}

static void create_buffer(Context *ctx, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    EXPECT(vkCreateBuffer(ctx->device, &bufferInfo, ctx->allocator, &buffer), "Failed to create buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(ctx->device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(ctx, memRequirements.memoryTypeBits, properties);

    EXPECT(vkAllocateMemory(ctx->device, &allocInfo, ctx->allocator, &bufferMemory), "Failed to allocate buffer memory!");

    vkBindBufferMemory(ctx->device, buffer, bufferMemory, 0);
}

void create_texture_image(State *state)
{
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load("assets/textures/wall.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    EXPECT(!pixels, "failed to load texture image!");

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    create_buffer(&state->context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(state->context.device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(state->context.device, stagingBufferMemory);

    stbi_image_free(pixels);

    create_image(&state->context, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, state->renderer.textureImage, state->renderer.textureImageMemory);

    transition_image_layout(state, state->renderer.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copy_buffer_to_image(state, stagingBuffer, state->renderer.textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    transition_image_layout(state, state->renderer.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(state->context.device, stagingBuffer, state->context.allocator);
    vkFreeMemory(state->context.device, stagingBufferMemory, state->context.allocator);
}

void create_texture_image_view(State *state)
{
    state->renderer.textureImageView = create_image_view(&state->context, state->renderer.textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void create_texture_sampler(State *state)
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    EXPECT(vkCreateSampler(state->context.device, &samplerInfo, state->context.allocator, &state->renderer.textureSampler), "failed to create texture sampler!");
}

void create_command_pool(State *state)
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = state->context.queueFamily;

    EXPECT(vkCreateCommandPool(state->context.device, &poolInfo, state->context.allocator, &state->renderer.commandPool), "Failed to create command pool!");
}

void create_command_buffers(State *state)
{
    state->renderer.commandBuffers = (VkCommandBuffer *)malloc(sizeof(VkCommandBuffer) * state->renderer.imageCount);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = state->renderer.commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = state->renderer.imageCount;

    EXPECT(vkAllocateCommandBuffers(state->context.device, &allocInfo, state->renderer.commandBuffers), "Failed to allocate command buffers!");
}

void create_sync_objects(State *state)
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        EXPECT(vkCreateSemaphore(state->context.device, &semaphoreInfo, state->context.allocator, &state->renderer.imageAvailableSemaphores[i]), "Failed to create semaphore!");
        EXPECT(vkCreateSemaphore(state->context.device, &semaphoreInfo, state->context.allocator, &state->renderer.renderFinishedSemaphores[i]), "Failed to create semaphore!");
        EXPECT(vkCreateFence(state->context.device, &fenceInfo, state->context.allocator, &state->renderer.inFlightFences[i]), "Failed to create fence!");
    }
}

void create_descriptor_set_layout(State *state)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::vector<VkDescriptorSetLayoutBinding> bindings = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    EXPECT(vkCreateDescriptorSetLayout(state->context.device, &layoutInfo, state->context.allocator, &state->renderer.descriptorSetLayout), "Failed to create descriptor set layout!");
}

void create_uniform_buffers(State *state)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        create_buffer(&state->context, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, state->renderer.uniformBuffers[i], state->renderer.uniformBuffersMemory[i]);
        vkMapMemory(state->context.device, state->renderer.uniformBuffersMemory[i], 0, bufferSize, 0, &state->renderer.uniformBuffersMapped[i]);
    }
}

void create_descriptor_pool(State *state)
{
    std::vector<VkDescriptorPoolSize> poolSizes(2);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    EXPECT(vkCreateDescriptorPool(state->context.device, &poolInfo, state->context.allocator, &state->renderer.descriptorPool), "Failed to create descriptor pool!");
}

void create_descriptor_sets(State *state)
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, state->renderer.descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = state->renderer.descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    EXPECT(vkAllocateDescriptorSets(state->context.device, &allocInfo, state->renderer.descriptorSets), "Failed to allocate descriptor sets!");

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = state->renderer.uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = state->renderer.textureImageView;
        imageInfo.sampler = state->renderer.textureSampler;

        std::vector<VkWriteDescriptorSet> descriptorWrites(2);

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = state->renderer.descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = state->renderer.descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(state->context.device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

uint32_t load_model(State *state, const char *path)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

    EXPECT(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode, "failed to load model: %s", importer.GetErrorString());

    uint32_t startingMeshIndex = (uint32_t)state->renderer.meshes.size();

    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[i];
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            Vertex vertex{};
            vertex.pos = vec3(mesh->mVertices[j].x, mesh->mVertices[j].y, mesh->mVertices[j].z);

            if (mesh->mNormals)
            {
                vertex.normal = vec3(mesh->mNormals[j].x, mesh->mNormals[j].y, mesh->mNormals[j].z);
            }

            if (mesh->mTextureCoords[0])
            {
                vertex.texCoord = vec2(mesh->mTextureCoords[0][j].x, mesh->mTextureCoords[0][j].y);
            }
            else
            {
                vertex.texCoord = vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        for (unsigned int j = 0; j < mesh->mNumFaces; j++)
        {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++)
            {
                indices.push_back(face.mIndices[k]);
            }
        }

        Mesh m = {};
        m.indexCount = static_cast<uint32_t>(indices.size());
        m.vertexCount = static_cast<uint32_t>(vertices.size());

        // Create vertex buffer
        {
            VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            create_buffer(&state->context, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

            void *data;
            vkMapMemory(state->context.device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, vertices.data(), (size_t)bufferSize);
            vkUnmapMemory(state->context.device, stagingBufferMemory);

            create_buffer(&state->context, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m.vertexBuffer, m.vertexBufferMemory);
            copy_buffer(state, stagingBuffer, m.vertexBuffer, bufferSize);

            vkDestroyBuffer(state->context.device, stagingBuffer, state->context.allocator);
            vkFreeMemory(state->context.device, stagingBufferMemory, state->context.allocator);
        }

        // Create index buffer
        {
            VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
            create_buffer(&state->context, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

            void *data;
            vkMapMemory(state->context.device, stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, indices.data(), (size_t)bufferSize);
            vkUnmapMemory(state->context.device, stagingBufferMemory);

            create_buffer(&state->context, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m.indexBuffer, m.indexBufferMemory);
            copy_buffer(state, stagingBuffer, m.indexBuffer, bufferSize);

            vkDestroyBuffer(state->context.device, stagingBuffer, state->context.allocator);
            vkFreeMemory(state->context.device, stagingBufferMemory, state->context.allocator);
        }

        state->renderer.meshes.push_back(m);
    }

    return (uint32_t)state->renderer.meshes.size() - startingMeshIndex;
}

static VkShaderModule create_shader_module(Context *ctx, const char *path)
{
    FILE *file = fopen(path, "rb");
    EXPECT(!file, "Failed to open shader file: %s", path);

    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(fileSize);
    EXPECT(!buffer, "Failed to allocate memory for shader: %s", path);

    size_t readSize = fread(buffer, 1, fileSize, file);
    EXPECT(readSize != fileSize, "Failed to read shader file: %s", path);

    fclose(file);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = fileSize;
    createInfo.pCode = reinterpret_cast<const uint32_t *>(buffer);

    VkShaderModule shaderModule;
    EXPECT(vkCreateShaderModule(ctx->device, &createInfo, ctx->allocator, &shaderModule), "Failed to create shader module: %s", path);

    free(buffer);

    return shaderModule;
}

void create_graphics_pipeline(State *state)
{
    VkShaderModule vertShaderModule = create_shader_module(&state->context, "shaders/shader.vert.spv");
    VkShaderModule fragShaderModule = create_shader_module(&state->context, "shaders/shader.frag.spv");

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(mat4);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &state->renderer.descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    EXPECT(vkCreatePipelineLayout(state->context.device, &pipelineLayoutInfo, state->context.allocator, &state->renderer.pipelineLayout), "Failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = state->renderer.pipelineLayout;
    pipelineInfo.renderPass = state->renderer.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    EXPECT(vkCreateGraphicsPipelines(state->context.device, VK_NULL_HANDLE, 1, &pipelineInfo, state->context.allocator, &state->renderer.graphicsPipeline), "Failed to create graphics pipeline!");

    vkDestroyShaderModule(state->context.device, fragShaderModule, state->context.allocator);
    vkDestroyShaderModule(state->context.device, vertShaderModule, state->context.allocator);
}

void record_command_buffer(State *state, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    EXPECT(vkBeginCommandBuffer(state->renderer.commandBuffers[state->renderer.currentFrame], &beginInfo), "Failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = state->renderer.renderPass;
    renderPassInfo.framebuffer = state->renderer.framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = state->renderer.extent;

    VkClearValue clearValues[2]{};
    clearValues[0].color = {{0.1f, 0.1f, 0.2f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(state->renderer.commandBuffers[state->renderer.currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(state->renderer.commandBuffers[state->renderer.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->renderer.graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)state->renderer.extent.width;
    viewport.height = (float)state->renderer.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(state->renderer.commandBuffers[state->renderer.currentFrame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = state->renderer.extent;
    vkCmdSetScissor(state->renderer.commandBuffers[state->renderer.currentFrame], 0, 1, &scissor);

    vkCmdBindDescriptorSets(state->renderer.commandBuffers[state->renderer.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, state->renderer.pipelineLayout, 0, 1, &state->renderer.descriptorSets[state->renderer.currentFrame], 0, nullptr);

    // Render Entities
    auto *meshPool = state->world.GetPool<MeshComponent>();
    auto *transformPool = state->world.GetPool<TransformComponent>();
    if (meshPool && transformPool)
    {
        for (size_t i = 0; i < meshPool->data.size(); i++)
        {
            ecs::EntityID entity = meshPool->denseToEntity[i];
            if (transformPool->Has(entity))
            {
                TransformComponent& transform = transformPool->Get(entity);
                uint32_t meshIdx = meshPool->data[i].meshIndex;
                if (meshIdx < state->renderer.meshes.size())
                {
                    const auto &mesh = state->renderer.meshes[meshIdx];
                    
                    // Calculate model matrix: M = T * R * S
                    mat4 model = mat4::Translate(transform.position) * 
                                 mat4::Rotate(quat::FromEuler(transform.rotation)) * 
                                 mat4::Scale(transform.scale);
                    
                    vkCmdPushConstants(state->renderer.commandBuffers[state->renderer.currentFrame], state->renderer.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat4), &model);

                    VkBuffer vertexBuffers[] = {mesh.vertexBuffer};
                    VkDeviceSize offsets[] = {0};
                    vkCmdBindVertexBuffers(state->renderer.commandBuffers[state->renderer.currentFrame], 0, 1, vertexBuffers, offsets);
                    vkCmdBindIndexBuffer(state->renderer.commandBuffers[state->renderer.currentFrame], mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
                    vkCmdDrawIndexed(state->renderer.commandBuffers[state->renderer.currentFrame], mesh.indexCount, 1, 0, 0, 0);
                }
            }
        }
    }

    vkCmdEndRenderPass(state->renderer.commandBuffers[state->renderer.currentFrame]);

    EXPECT(vkEndCommandBuffer(state->renderer.commandBuffers[state->renderer.currentFrame]), "Failed to record command buffer!");
}

void update_uniform_buffer(State *state)
{
    UniformBufferObject ubo{};
    
    // View matrix: camera at (0,0,5) looking at origin
    ubo.view = mat4::Translate(vec3(0, 0, -5));

    // Projection matrix
    float aspect = state->renderer.extent.width / (float)state->renderer.extent.height;
    ubo.proj = mat4::Perspective(45.0f * (3.14159265f / 180.0f), aspect, 0.1f, 100.0f);
    
    // Vulkan Y is flipped
    ubo.proj.m[5] *= -1;

    memcpy(state->renderer.uniformBuffersMapped[state->renderer.currentFrame], &ubo, sizeof(ubo));
}

void draw_frame(State *state)
{
    vkWaitForFences(state->context.device, 1, &state->renderer.inFlightFences[state->renderer.currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(state->context.device, state->renderer.swapchain, UINT64_MAX, state->renderer.imageAvailableSemaphores[state->renderer.currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreate_swapchain(state);
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        EXPECT(true, "failed to acquire swap chain image!");
    }

    vkResetFences(state->context.device, 1, &state->renderer.inFlightFences[state->renderer.currentFrame]);

    update_uniform_buffer(state);

    vkResetCommandBuffer(state->renderer.commandBuffers[state->renderer.currentFrame], 0);
    record_command_buffer(state, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {state->renderer.imageAvailableSemaphores[state->renderer.currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &state->renderer.commandBuffers[state->renderer.currentFrame];

    VkSemaphore signalSemaphores[] = {state->renderer.renderFinishedSemaphores[state->renderer.currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    EXPECT(vkQueueSubmit(state->context.queue, 1, &submitInfo, state->renderer.inFlightFences[state->renderer.currentFrame]), "Failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {state->renderer.swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(state->context.queue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || state->framebufferResized)
    {
        state->framebufferResized = false;
        recreate_swapchain(state);
    }
    else if (result != VK_SUCCESS)
    {
        EXPECT(true, "failed to present swap chain image!");
    }

    state->renderer.currentFrame = (state->renderer.currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void destroy_graphic(State *state)
{
    if (state->renderer.textureSampler != VK_NULL_HANDLE)
        vkDestroySampler(state->context.device, state->renderer.textureSampler, state->context.allocator);
    if (state->renderer.textureImageView != VK_NULL_HANDLE)
        vkDestroyImageView(state->context.device, state->renderer.textureImageView, state->context.allocator);
    if (state->renderer.textureImage != VK_NULL_HANDLE)
        vkDestroyImage(state->context.device, state->renderer.textureImage, state->context.allocator);
    if (state->renderer.textureImageMemory != VK_NULL_HANDLE)
        vkFreeMemory(state->context.device, state->renderer.textureImageMemory, state->context.allocator);

    if (state->renderer.depthImageView != VK_NULL_HANDLE)
        vkDestroyImageView(state->context.device, state->renderer.depthImageView, state->context.allocator);
    if (state->renderer.depthImage != VK_NULL_HANDLE)
        vkDestroyImage(state->context.device, state->renderer.depthImage, state->context.allocator);
    if (state->renderer.depthImageMemory != VK_NULL_HANDLE)
        vkFreeMemory(state->context.device, state->renderer.depthImageMemory, state->context.allocator);

    if (state->renderer.descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(state->context.device, state->renderer.descriptorSetLayout, state->context.allocator);
    }

    if (state->renderer.descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(state->context.device, state->renderer.descriptorPool, state->context.allocator);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (state->renderer.uniformBuffers[i] != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(state->context.device, state->renderer.uniformBuffers[i], state->context.allocator);
            vkFreeMemory(state->context.device, state->renderer.uniformBuffersMemory[i], state->context.allocator);
        }
    }

    for (auto &mesh : state->renderer.meshes)
    {
        if (mesh.indexBuffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(state->context.device, mesh.indexBuffer, state->context.allocator);
            vkFreeMemory(state->context.device, mesh.indexBufferMemory, state->context.allocator);
        }
        if (mesh.vertexBuffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(state->context.device, mesh.vertexBuffer, state->context.allocator);
            vkFreeMemory(state->context.device, mesh.vertexBufferMemory, state->context.allocator);
        }
    }
    state->renderer.meshes.clear();

    if (state->renderer.graphicsPipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(state->context.device, state->renderer.graphicsPipeline, state->context.allocator);
    }
    if (state->renderer.pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(state->context.device, state->renderer.pipelineLayout, state->context.allocator);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(state->context.device, state->renderer.imageAvailableSemaphores[i], state->context.allocator);
        vkDestroySemaphore(state->context.device, state->renderer.renderFinishedSemaphores[i], state->context.allocator);
        vkDestroyFence(state->context.device, state->renderer.inFlightFences[i], state->context.allocator);
    }

    if (state->renderer.commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(state->context.device, state->renderer.commandPool, state->context.allocator);
    }
    if (state->renderer.commandBuffers != nullptr)
    {
        free(state->renderer.commandBuffers);
    }

    for (uint32_t i = 0; i < state->renderer.imageCount; i++)
    {
        if (state->renderer.framebuffers != nullptr)
        {
            vkDestroyFramebuffer(state->context.device, state->renderer.framebuffers[i], state->context.allocator);
        }
    }
    if (state->renderer.framebuffers != nullptr)
    {
        free(state->renderer.framebuffers);
    }

    if (state->renderer.renderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(state->context.device, state->renderer.renderPass, state->context.allocator);
    }
}