#pragma once
#include <utils.h>

constexpr int RENDERING_OPTION_FLIP_X = BIT(0);
constexpr int RENDERING_OPTION_FLIP_Y = BIT(1);
constexpr int RENDERING_OPTION_FONT = BIT(2);

struct DrawData
{
    int anim_x;
    int renderOptions;
    float layer = 0.0f;
};

struct alignas(16) Transform
{
    ivec2 ioffset;     // 8 bytes
    ivec2 isize;       // 8 bytes
    vec2 pos;          // 8 bytes
    vec2 size;         // 8 bytes
    vec4 color;        // 16 bytes (Starts at byte 32)
    int renderOptions; // 4 bytes  (Starts at byte 48)
    float layer;       // 
    int _padding[2];   // was 3, now 2
};

struct Camera2D
{
    vec2 pos;
    // `dimensions` is the logical view size (world units) used for projection.
    // It stays at the default (initial) window size so the camera's logical
    // coordinate system remains stable when the window is resized.
    vec2 dimensions;

    // `framebuffer` stores the actual drawable pixel size (framebuffer or
    // client size). This is used for `glViewport` so the GL viewport matches
    // the window pixel dimensions while the projection uses `dimensions`.
    vec2 framebuffer;
    mat4 matrix()
    {
        mat4 p;
        p = Mat4::Ortho(0, dimensions.x, dimensions.y, 0, -1, 1);
        return p;
    }
};

struct RenderData
{
    Camera2D camera;
    Array<Transform> transforms;
    Array<Transform> uiTransforms;
    void OnResize(int x, int y)
    {
        // If the logical camera has not been initialized (zero), use the
        // first resize (typically window creation) to set the logical view
        // size. On subsequent resizes we only update the framebuffer size so
        // the logical camera remains fixed and content scales with the
        // viewport.
        if (camera.dimensions.x == 0.0f && camera.dimensions.y == 0.0f)
        {
            camera.dimensions = {(float)x, (float)y};
        }
        camera.framebuffer = {(float)x, (float)y};
    }
};

struct Glyph
{
    ivec2 size;    // glyph bitmap size
    ivec2 bearing; // offset from baseline
    u32 advance;   // advance.x from FreeType
    ivec2 offset;  // atlas position (ioffset)
};

extern Input *input;
extern RenderData *renderData;