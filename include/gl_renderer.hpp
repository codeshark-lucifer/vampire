#pragma once

#include <shader.h>
#include <render_types.h>
#include <map>
#include <string>

constexpr str FONT_FILE_PATH     = "assets/fonts/arial.ttf";
constexpr str SHADER_SCENE_VERT  = "assets/shaders/scene.vert";
constexpr str SHADER_SCENE_FRAG  = "assets/shaders/scene.frag";
constexpr str ATLAS_TEXTURE_PATH = "assets/textures/sample.png";

struct GLContext
{
    Shader* shader;
    u32 vao;
    u32 transSSBO;
    u32 textureAtlas;
    ivec2 textureSize;
};

struct Font
{
    std::map<char, Glyph> glyphs;
    u32 texture;
    int atlasWidth;
    int atlasHeight;
    int pixelSize;
    int ascender; // in pixels (face->size->metrics.ascender >> 6)
};

extern GLContext gl;
extern Font font;

// Public API
bool glInit(BumpAllocator* persistentStorage);
void glRender();

void PushSprite(ivec2 offset, ivec2 size, vec2 pos, vec2 renderSize, vec3 color, int renderOptions, float layer);
void DrawUIText(const std::string& text, vec2 pos, float scale, vec3 color);
