#version 430 core

struct Transform {
    ivec2 ioffset;
    ivec2 isize;
    vec2 pos;
    vec2 size;
    vec4 color;
    int renderOptions; 
    float layer;
};

// Define the same flags as C++
const int FLIP_X = 1; // BIT(0)
const int FLIP_Y = 2; // BIT(1)

layout (location = 0) out vec2 textureCoordsOut;
layout (location = 1) out vec4 baseColor;

layout (std430, binding = 0) buffer TransformSBO {
    Transform transforms[];
};

uniform mat4 projection;
uniform vec2 atlasSize;

void main()
{
    Transform transform = transforms[gl_InstanceID];
    
    // 1. Calculate base vertices (same as before)
    vec2 vertices[6] = {
        transform.pos,
        vec2(transform.pos + vec2(0.0, transform.size.y)),
        vec2(transform.pos + vec2(transform.size.x, 0.0)),
        vec2(transform.pos + vec2(transform.size.x, 0.0)),
        vec2(transform.pos + vec2(0.0, transform.size.y)),
        transform.pos + transform.size
    };

    // 2. Handle Texture Coordinate Logic
    int left   = transform.ioffset.x;
    int top    = transform.ioffset.y;
    int right  = transform.ioffset.x + transform.isize.x;
    int bottom = transform.ioffset.y + transform.isize.y;

    // Check bits for Flipping
    if ((transform.renderOptions & FLIP_X) != 0) {
        int temp = left;
        left = right;
        right = temp;
    }
    if ((transform.renderOptions & FLIP_Y) != 0) {
        int temp = top;
        top = bottom;
        bottom = temp;
    }

    vec2 textureCoords[6] = {
        vec2(left,  top)    / atlasSize,
        vec2(left,  bottom) / atlasSize,
        vec2(right, top)    / atlasSize,
        vec2(right, top)    / atlasSize,
        vec2(left,  bottom) / atlasSize,
        vec2(right, bottom) / atlasSize
    };

    baseColor = transform.color;
    textureCoordsOut = textureCoords[gl_VertexID];
    gl_Position = projection * vec4(vertices[gl_VertexID], transform.layer, 1.0);
}