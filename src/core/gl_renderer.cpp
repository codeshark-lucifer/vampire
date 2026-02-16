#include <gl_renderer.hpp>

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>

GLContext gl;
Font font;

extern RenderData *renderData;
extern Input *input;

void PushSprite(ivec2 offset, ivec2 size, vec2 pos, vec2 renderSize, vec3 color, int renderOptions, float layer)
{
    Transform trans{};
    trans.ioffset = offset;
    trans.isize = size;
    trans.pos = pos;
    trans.size = renderSize;
    trans.color = vec4(color, 1.0f);
    trans.renderOptions = renderOptions;
    trans.layer = layer;

    renderData->transforms.push_back(trans);
}

bool LoadFont(const char *path, int pixelSize)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
        return false;

    FT_Face face;
    if (FT_New_Face(ft, path, 0, &face))
        return false;

    FT_Set_Pixel_Sizes(face, 0, pixelSize);

    // store ascender (in pixels) for baseline calculations
    font.pixelSize = pixelSize;
    if (face->size)
        font.ascender = (face->size->metrics.ascender >> 6);
    else
        font.ascender = pixelSize; // fallback

    const int firstChar = 32;
    const int lastChar = 126;

    int atlasWidth = 0;
    int atlasHeight = 0;

    for (int c = firstChar; c <= lastChar; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            continue;

        atlasWidth += face->glyph->bitmap.width;
        atlasHeight = std::max(atlasHeight,
                               (int)face->glyph->bitmap.rows);
    }

    font.atlasWidth = atlasWidth;
    font.atlasHeight = atlasHeight;
    font.pixelSize = pixelSize;

    std::vector<unsigned char> atlas(atlasWidth * atlasHeight);

    int x = 0;

    for (int c = firstChar; c <= lastChar; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            continue;

        FT_Bitmap &bmp = face->glyph->bitmap;

        for (int row = 0; row < bmp.rows; row++)
            for (int col = 0; col < bmp.width; col++)
            {
                int index = row * atlasWidth + (x + col);
                atlas[index] = bmp.buffer[row * bmp.pitch + col];
            }

        Glyph glyph;
        glyph.size = ivec2(bmp.width, bmp.rows);
        glyph.bearing = ivec2(face->glyph->bitmap_left,
                              face->glyph->bitmap_top);
        glyph.advance = face->glyph->advance.x;
        glyph.offset = ivec2(x, 0);

        font.glyphs.insert({(char)c, glyph});

        x += bmp.width;
    }

    glGenTextures(1, &font.texture);
    glBindTexture(GL_TEXTURE_2D, font.texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                 atlasWidth, atlasHeight,
                 0, GL_RED, GL_UNSIGNED_BYTE,
                 atlas.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return true;
}

bool glInit(BumpAllocator *persistentStorage)
{
    gl.shader = BumpAlloc<Shader>(
        persistentStorage,
        SHADER_SCENE_VERT,
        SHADER_SCENE_FRAG);

    glGenBuffers(1, &gl.transSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl.transSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gl.transSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    int ch;
    unsigned char *image =
        stbi_load(ATLAS_TEXTURE_PATH,
                  &gl.textureSize.x,
                  &gl.textureSize.y,
                  &ch,
                  STBI_rgb_alpha);

    if (!image)
        return false;

    glGenTextures(1, &gl.textureAtlas);
    glBindTexture(GL_TEXTURE_2D, gl.textureAtlas);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 gl.textureSize.x, gl.textureSize.y,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);

    LoadFont(FONT_FILE_PATH, 24);

    gl.shader->Use();
    gl.shader->SetUniform("textureAtlas", 0);

    glGenVertexArrays(1, &gl.vao);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void glRender()
{
    Camera2D &camera = renderData->camera;
    mat4 projection = camera.matrix();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // Use the actual framebuffer/client size for the GL viewport so the
    // projection (which uses the logical `camera.dimensions`) remains fixed
    // while the viewport scales to the window size.
    vec2 fb = camera.framebuffer;
    if (fb.x <= 0.0f || fb.y <= 0.0f)
    {
        // Fallback to logical dimensions if framebuffer wasn't set.
        fb = camera.dimensions;
    }
    glViewport(0, 0, (int)fb.x, (int)fb.y);
    
    gl.shader->Use();
    gl.shader->SetUniform("projection", projection);
    glBindVertexArray(gl.vao);

    // 1. Render Sprites
    if (!renderData->transforms.empty())
    {
        gl.shader->SetUniform("atlasSize", vec2(gl.textureSize.x, gl.textureSize.y));
        gl.shader->SetUniform("isFont", 0); // Use int for bool uniforms often safer

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl.transSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Transform) * renderData->transforms.size(), renderData->transforms.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gl.transSSBO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gl.textureAtlas);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)renderData->transforms.size());
    }

    // 2. Render Text
    if (!renderData->uiTransforms.empty())
    {
        gl.shader->SetUniform("atlasSize", vec2(font.atlasWidth, font.atlasHeight));
        gl.shader->SetUniform("isFont", 1);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl.transSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Transform) * renderData->uiTransforms.size(), renderData->uiTransforms.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gl.transSSBO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, font.texture);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, (GLsizei)renderData->uiTransforms.size());
    }

    renderData->transforms.clear();
    renderData->uiTransforms.clear();
}