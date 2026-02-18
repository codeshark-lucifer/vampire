#pragma once
#include <utils.h>

enum SpriteID
{
    SPRITE_WHITE,
    SPRITE_REDBALL,
    SPRITE_TILETEMP,
    SPRITE_TILETEMP1,
    SPRITE_COIN,

    SPRITE_COUNT
};

struct Sprite
{
    ivec2 offset;
    ivec2 size;
    int frameCount;
};

inline Sprite getSprite(SpriteID spriteID)
{
    Sprite sprite = {};
    sprite.frameCount = 1;

    switch (spriteID)
    {
    case SPRITE_WHITE:
    {
        sprite.offset = {0, 0};
        sprite.size = {1, 1};
        break;
    }

    case SPRITE_REDBALL:
    {
        sprite.offset = {16, 0};
        sprite.size = {16, 16};
        break;
    }

    case SPRITE_TILETEMP:
    {
        sprite.offset = {32, 0};
        sprite.size = {32, 32};
        break;
    }
    case SPRITE_TILETEMP1:
    {
        sprite.offset = {64, 0};
        sprite.size = {32, 32};
        break;
    }
    default:
        return getSprite(SPRITE_WHITE);
    }

    return sprite;
}