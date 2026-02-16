#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <utils/dtype.h>

#include <utils/macros.h>
#include <utils/mathf.h>
#include <utils/color.h>
#include <utils/keycode.h>

#ifdef _WIN32
#include <sys/stat.h>
#endif

#include <vector>

typedef IVec2 ivec2;
typedef Vec2 vec2;
typedef Vec3 vec3;
typedef Vec4 vec4;
typedef Mat3 mat3;
typedef Mat4 mat4;
typedef Quat quat;

template <typename T>
using Array = std::vector<T>;

enum MouseMode
{
    MOUSE_VISIBLE,
    MOUSE_HIDDEN,
    MOUSE_LOCKED
};

struct Input
{
    ivec2 screenSize{0, 0};
    MouseMode mouseMode = MOUSE_VISIBLE;

    // Keyboard
    bool keys[256]{false};     // current frame key states
    bool keysPrev[256]{false}; // previous frame states

    // Mouse
    vec2 mousePosScreen{0, 0};   // pixel coordinates
    vec2 mousePos{0, 0};         // maybe world coordinates later
    bool mouseButtons[5]{false}; // left, right, middle, extra1, extra2
    bool mouseButtonsPrev[5]{false};

    // Call every frame to store previous states
    void UpdatePrev()
    {
        memcpy(keysPrev, keys, sizeof(keys));
        memcpy(mouseButtonsPrev, mouseButtons, sizeof(mouseButtons));
    }

    // Keyboard checks
    bool IsKeyHeld(int vk) const { return keys[vk]; }
    bool IsKeyPressed(int vk) const { return keys[vk] && !keysPrev[vk]; }
    bool IsKeyReleased(int vk) const { return !keys[vk] && keysPrev[vk]; }

    // Mouse checks
    bool IsMouseHeld(int button) const { return mouseButtons[button]; }
    bool IsMousePressed(int button) const { return mouseButtons[button] && !mouseButtonsPrev[button]; }
    bool IsMouseReleased(int button) const { return !mouseButtons[button] && mouseButtonsPrev[button]; }
};

struct BumpAllocator
{
    size_t capacity;
    size_t used;
    char *memory;
};

long long get_timestamp(const char *filepath);
char *read_file(str filepath, int *filesize);
bool write_file(str filepath, const void *data, int size);

BumpAllocator MakeAllocator(size_t size);
void *BumpAllocAligned(BumpAllocator *alloc, size_t size, size_t align);

template <typename T, typename... Args>
T *BumpAlloc(BumpAllocator *alloc, Args &&...args)
{
    void *memory = BumpAllocAligned(alloc, sizeof(T), alignof(T));
    LOG_ASSERT(memory, "BumpAlloc out of memory");

    return new (memory) T(std::forward<Args>(args)...);
}

inline int animate(float* time, int frameCount, float duration = 1.0f)
{
  while(*time > duration)
  {
    *time -= duration;
  }
  
  int animationIdx = (int)((*time / duration) * frameCount);
  
  // Clamp
  if (animationIdx >= frameCount)
  {
    animationIdx = frameCount - 1;
  }
  
  return animationIdx;
}

inline float approach(float current, float target, float amount)
{
    if (current < target)
    {
        current += amount;
        if (current > target) return target;
    }
    else
    {
        current -= amount;
        if (current < target) return target;
    }
    return current;
}