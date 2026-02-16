#include "platform/win32.h"

#include <windows.h>
#include <windowsx.h>
// #include <GL/gl.h>
#include <glad/glad.h>
#include <GL/wglext.h>

#include <dwmapi.h>
#include <utils.h>
#include <render_types.h>

BumpAllocator persistentStorage;

const char *WINDOW_CLASS = "AtlasClass";
bool running = false;

HDC hdc = NULL;
HGLRC hrc = NULL;
Window window = NULL;
HINSTANCE hInstance = NULL;

bool keys[256] = {};
bool active = true;
bool fullscreen = false;
float deltaTime = 0.0f;

static UINT GetWindowDPI(HWND hwnd)
{
    auto user32 = GetModuleHandleA("user32.dll");
    if (user32)
    {
        typedef UINT(WINAPI * GetDpiForWindow_t)(HWND);
        auto pGetDpiForWindow = (GetDpiForWindow_t)GetProcAddress(user32, "GetDpiForWindow");
        if (pGetDpiForWindow)
            return pGetDpiForWindow(hwnd);
    }
    return 96; // Default DPI
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    // Keyboard
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (wParam < 256)
            input->keys[wParam] = true;
        break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (wParam < 256)
            input->keys[wParam] = false;
        break;

    // Mouse
    case WM_MOUSEMOVE:
    {
        if (input && renderData)
        {
            RECT cr;
            GetClientRect(hwnd, &cr);
            float clientW = (float)(cr.right - cr.left);
            float clientH = (float)(cr.bottom - cr.top);

            float mouseX = (float)GET_X_LPARAM(lParam);
            float mouseY = (float)GET_Y_LPARAM(lParam);

            // Normalize 0.0 -> 1.0
            float normX = mouseX / clientW;
            float normY = mouseY / clientH;

            // Map to logical camera dimensions (the units used in your Ortho matrix)
            input->mousePosScreen.x = normX * renderData->camera.dimensions.x;
            input->mousePosScreen.y = normY * renderData->camera.dimensions.y;
        }
        break;
    }

    case WM_LBUTTONDOWN:
        input->mouseButtons[0] = true;
        break;
    case WM_LBUTTONUP:
        input->mouseButtons[0] = false;
        break;
    case WM_RBUTTONDOWN:
        input->mouseButtons[1] = true;
        break;
    case WM_RBUTTONUP:
        input->mouseButtons[1] = false;
        break;
    case WM_MBUTTONDOWN:
        input->mouseButtons[2] = true;
        break;
    case WM_MBUTTONUP:
        input->mouseButtons[2] = false;
        break;
    case WM_XBUTTONDOWN:
        input->mouseButtons[GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 3 : 4] = true;
        break;
    case WM_XBUTTONUP:
        input->mouseButtons[GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 3 : 4] = false;
        break;
    case WM_ACTIVATE:
    {
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            ClipCursor(NULL);
            while (ShowCursor(TRUE) < 0)
                ;
        }
        break;
    }

    case WM_SIZE:
    {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        UINT dpi = GetWindowDPI(hwnd);
        float scale = (float)dpi / 96.0f;

        int pixelW = (int)(width * scale);
        int pixelH = (int)(height * scale);

        // if (input)
        //     input->screenSize = {pixelW, pixelH}; // Removed (float) casts
        if (renderData)
            renderData->OnResize(pixelW, pixelH);
        return 0;
    }
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

bool ShouldClose()
{
    return !running;
}

bool InitPlatform()
{
    persistentStorage = MakeAllocator(MB(100));
    input = BumpAlloc<Input>(&persistentStorage);
    renderData = BumpAlloc<RenderData>(&persistentStorage);
    if (!gladLoadGL())
        return false;
    return true;
}

void PollEvent(Event *event)
{
    if (PeekMessage(&event->msg, NULL, 0, 0, PM_REMOVE))
    {
        if (event->msg.message == WM_QUIT)
            running = false;

        TranslateMessage(&event->msg);
        DispatchMessage(&event->msg); // This calls WndProc for you!
    }

    // Time tracking is fine here
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = now - lastTime;
    deltaTime = elapsed.count();
    lastTime = now;

    // DELETE the switch statement that was here.
}

void SwapBuffersWindow()
{
    SwapBuffers(hdc);
}

Window CreateWindowPlatform(int width, int height, const char *name)
{
    // input->screenSize = {width, height}; // Initial set based on creation params, will be overwritten by default game resolution

    // Set input->screenSize to the fixed internal game resolution
    input->screenSize = {(int)DEFAULT_GAME_WIDTH, (int)DEFAULT_GAME_HEIGHT};
    hInstance = GetModuleHandleA(NULL);

    WNDCLASSA wc = {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_CLASS;
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    RegisterClassA(&wc);

    window = CreateWindowExA(
        0, wc.lpszClassName, name,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        100, 100, width, height,
        NULL, NULL, hInstance, NULL);

    // Ensure we use the actual client area size (accounting for window borders / DPI)
    RECT crect;
    GetClientRect(window, &crect);
    int w = crect.right - crect.left;
    int h = crect.bottom - crect.top;

    UINT dpi = 96;
    auto user32 = GetModuleHandleA("user32.dll");
    if (user32)
    {
        typedef UINT(WINAPI * GetDpiForWindow_t)(HWND);
        GetDpiForWindow_t pGetDpiForWindow = (GetDpiForWindow_t)GetProcAddress(user32, "GetDpiForWindow");
        if (pGetDpiForWindow)
            dpi = pGetDpiForWindow(window);
    }
    float scale = (float)dpi / 96.0f;
    int clientW = (int)roundf(w * scale);
    int clientH = (int)roundf(h * scale);

    // if (input)
    //     input->screenSize = {clientW, clientH}; // input->screenSize should represent the internal game resolution, not the window size
    if (renderData)
        renderData->OnResize(clientW, clientH);

    hdc = GetDC(window);

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;

    int pf = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pf, &pfd);

    HGLRC tempContext = wglCreateContext(hdc);
    wglMakeCurrent(hdc, tempContext);

    auto wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)
            wglGetProcAddress("wglCreateContextAttribsARB");

    if (!wglCreateContextAttribsARB)
    {
        MessageBoxA(0, "wglCreateContextAttribsARB not supported", "Error", MB_OK);
        exit(1);
    }

    int attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0};

    hrc = wglCreateContextAttribsARB(hdc, 0, attribs);

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(tempContext);
    wglMakeCurrent(hdc, hrc);

    if (!gladLoadGL())
    {
        MessageBoxA(0, "Failed to load OpenGL", "Error", MB_OK);
        exit(1);
    }

    running = true;
    return window;
}

void ShutdownPlatform()
{
    if (hrc)
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hrc);
        hrc = NULL;
    }

    if (window && hdc)
    {
        ReleaseDC(window, hdc);
        hdc = NULL;
    }

    if (window)
    {
        DestroyWindow(window);
        window = NULL;
    }

    UnregisterClassA(WINDOW_CLASS, hInstance);
}

void SetColorTitleBar(Color background, Color text = {1.0f, 1.0f, 1.0f, 1.0f})
{
    COLORREF bgColor = RGB((int)255 * background.r, (int)255 * background.g, (int)255 * background.b);
    COLORREF textColor = RGB((int)255 * text.r, (int)255 * text.g, (int)255 * text.b);

    DwmSetWindowAttribute(
        window,
        DWMWA_CAPTION_COLOR,
        &bgColor,
        sizeof(bgColor));

    DwmSetWindowAttribute(
        window,
        DWMWA_TEXT_COLOR,
        &textColor,
        sizeof(textColor));
}

void SetMouseMode(MouseMode mode)
{
    if (!window)
        return;

    input->mouseMode = mode;

    if (mode == MOUSE_VISIBLE)
    {
        while (ShowCursor(TRUE) < 0)
            ;
        ClipCursor(NULL);
    }
    else if (mode == MOUSE_HIDDEN)
    {
        while (ShowCursor(FALSE) >= 0)
            ;
        ClipCursor(NULL);
    }
    else if (mode == MOUSE_LOCKED)
    {
        while (ShowCursor(FALSE) >= 0)
            ;

        RECT rect;
        GetClientRect(window, &rect);

        POINT ul = {rect.left, rect.top};
        POINT lr = {rect.right, rect.bottom};

        ClientToScreen(window, &ul);
        ClientToScreen(window, &lr);

        rect.left = ul.x;
        rect.top = ul.y;
        rect.right = lr.x;
        rect.bottom = lr.y;

        ClipCursor(&rect);

        // Center cursor
        int centerX = (rect.left + rect.right) / 2;
        int centerY = (rect.top + rect.bottom) / 2;
        SetCursorPos(centerX, centerY);
    }
}
