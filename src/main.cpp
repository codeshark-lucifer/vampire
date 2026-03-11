#include <vulkan/vulkan.h>
#include <engine/utils.h>
#include <engine/graphic.h>
#include <ecs/ecs.h>

#include <vector>
#include <cstdio>
#include <cstdlib>


void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    auto state = reinterpret_cast<State *>(glfwGetWindowUserPointer(window));
    state->framebufferResized = true;
}

void init(State *s)
{
    glfwSetErrorCallback(callback_error);
    atexit(callback_exit);

    create_window(&s->config, &s->window);
    glfwSetWindowUserPointer(s->window.handle, s);
    glfwSetFramebufferSizeCallback(s->window.handle, framebuffer_size_callback);

    create_instance(&s->config, &s->context);

    // Log info
    u32 ver;
    vkEnumerateInstanceVersion(&ver);
    printf("*Start Engine:\n\t- Vulkan: %i.%i.%i \n\t- GLFW: %s\n", VK_API_VERSION_MAJOR(ver), VK_API_VERSION_MINOR(ver), VK_API_VERSION_PATCH(ver), glfwGetVersionString());

    select_physical_device(&s->context);
    EXPECT(glfwCreateWindowSurface(s->context.instance, s->window.handle, s->context.allocator, &s->context.surface), "Surface creation failed.");

    select_queue_family(&s->context);
    create_logical_device(&s->context);
    create_swapchain(&s->context, &s->renderer);

    create_command_pool(s);

    create_texture_image(s);
    create_texture_image_view(s);
    create_texture_sampler(s);

    uint32_t numMeshes = load_model(s, "assets/models/cube.obj");
    for (uint32_t i = 0; i < numMeshes; i++)
    {
        ecs::EntityID ent = s->world.CreateEntity();
        s->world.AddComponent<TransformComponent>(ent, {vec3(0, 0, 0), vec3(0, 45, 0), vec3(1, 1, 1)});
        s->world.AddComponent<MeshComponent>(ent, {i});
    }

    create_depth_resources(s);

    create_render_pass(s);
    create_descriptor_set_layout(s);
    create_graphics_pipeline(s);
    create_framebuffers(s);
    create_command_buffers(s);
    create_sync_objects(s);
    create_uniform_buffers(s);
    create_descriptor_pool(s);
    create_descriptor_sets(s);

    glfwSetKeyCallback(s->window.handle, callback_key);
}

void cleanup(State *s)
{
    if (s->context.device != VK_NULL_HANDLE)
        vkDeviceWaitIdle(s->context.device);

    destroy_graphic(s);

    if (s->renderer.views != nullptr)
    {
        for (u32 i = 0; i < s->renderer.imageCount; i++)
        {
            if (s->renderer.views[i] != VK_NULL_HANDLE)
            {
                vkDestroyImageView(s->context.device, s->renderer.views[i], s->context.allocator);
            }
        }
        free(s->renderer.views);
    }

    if (s->renderer.images != nullptr)
    {
        free(s->renderer.images);
    }

    if (s->renderer.swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(s->context.device, s->renderer.swapchain, s->context.allocator);
    }

    if (s->context.device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(s->context.device, s->context.allocator);
    }

    if (s->context.surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(s->context.instance, s->context.surface, s->context.allocator);
    }

    if (s->window.handle)
        glfwDestroyWindow(s->window.handle);

    if (s->context.enableValidationLayers && s->context.debugMessenger != VK_NULL_HANDLE)
    {
        destroy_debug_utils_messenger_ext(s->context.instance, s->context.debugMessenger, s->context.allocator);
    }

    if (s->context.instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(s->context.instance, s->context.allocator);
    }
}

int main()
{
    State state = {};
    state.config = {
        .application_name = "Vampire",
        .engine_name = "AtlasEngine",
        .window_title = "Vampire",
        .window_width = 956,
        .window_height = 540,
        .window_resizeable = true,
        .api_version = VK_API_VERSION_1_4};

    init(&state);

    while (!glfwWindowShouldClose(state.window.handle))
    {
        glfwPollEvents();
        draw_frame(&state);
    }

    vkDeviceWaitIdle(state.context.device);
    cleanup(&state);
    return EXIT_SUCCESS;
}
