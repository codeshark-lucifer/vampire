#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#if defined(_WIN32) || defined(_WIN64)
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct
{
    const char *application_name;
    const char *engine_name;
    const char *window_title;

    int window_width;
    int window_height;

    bool window_resizeable;
    bool window_fullscreen;

    uint32_t api_version;
} Config;

typedef struct
{
    GLFWwindow *handle;
    GLFWmonitor *monitor;
} Window;

void callback_key(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void callback_error(int code, const char *description)
{
    fprintf(stderr, "GLFW Error [%d]: %s\n", code, description);
}

void callback_exit()
{
    printf("\nExit Engine.\n");
    glfwTerminate();
}

void create_window(Config *cfg, Window *win)
{
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, cfg->window_resizeable ? GLFW_TRUE : GLFW_FALSE);

    if (cfg->window_fullscreen)
    {
        win->monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *mode = glfwGetVideoMode(win->monitor);

        cfg->window_width = mode->width;
        cfg->window_height = mode->height;
    }

    win->handle = glfwCreateWindow(
        cfg->window_width,
        cfg->window_height,
        cfg->window_title,
        win->monitor,
        NULL);

#if defined(_WIN32) || defined(_WIN64)

    HWND hwnd = glfwGetWin32Window(win->handle);

    BOOL dark = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));

    COLORREF title = RGB(30, 30, 30);
    DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &title, sizeof(title));

    COLORREF text = RGB(255, 255, 255);
    DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &text, sizeof(text));

#endif

    glfwSetKeyCallback(win->handle, callback_key);
}