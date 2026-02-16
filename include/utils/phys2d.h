#pragma once

// Make sure this path correctly points to where vec2/ivec2 are defined
// If they are in mathf.h, include that instead.
#include "vector.h" 

struct Rect {
    Vec2 pos; 
    Vec2 size;
};

// Use inline to prevent multiple definition errors during linking
inline bool CheckCollision(Rect a, Rect b) {
    return (a.pos.x < b.pos.x + b.size.x &&
            a.pos.x + a.size.x > b.pos.x &&
            a.pos.y < b.pos.y + b.size.y &&
            a.pos.y + a.size.y > b.pos.y);
}

inline Rect MakeRect(Vec2 pos, Vec2 size)
{
    Rect rect = { pos, size };
    return rect;
}
