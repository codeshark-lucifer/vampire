#pragma once
#include <windows.h>
#include <utils.h>
#include <chrono>

enum EventType
{
    EVENT_NONE,
    EVENT_KEY_DOWN,
    EVENT_KEY_UP,
    EVENT_MOUSE_MOVE,
    EVENT_MOUSE_BUTTON_DOWN,
    EVENT_MOUSE_BUTTON_UP,
    EVENT_QUIT
};

struct Event
{
    EventType type = EVENT_NONE;

    // Keyboard
    unsigned int key = 0;

    // Mouse
    int mouseX = 0;
    int mouseY = 0;
    int mouseButton = 0;

    // Original Win32 message
    MSG msg;
};

typedef HWND Window;

extern bool running;

extern HDC hdc;
extern HGLRC hrc;
extern Window window;
extern HINSTANCE hInstance;

extern bool keys[256];
extern bool active;
extern bool fullscreen;
extern float deltaTime;

extern BumpAllocator persistentStorage;

static auto lastTime = std::chrono::high_resolution_clock::now();
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

bool ShouldClose();
bool InitPlatform();
void PollEvent(Event* event);
void SwapBuffersWindow();
Window CreateWindowPlatform(int width, int height, const char* name);
void ShutdownPlatform();

void SetColorTitleBar(Color background, Color text);
void SetMouseMode(MouseMode mode);
