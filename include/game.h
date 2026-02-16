#pragma once
#include <assets.h>
#include <interface.h>
#include <platform/win32.h>
#include <systems/particle.h>

constexpr float FIXED_DELTATIME = 1.0f / 60.0f;

constexpr int TILESIZE = 32;
constexpr ivec2 GRID_WORLD = {3, 3};

struct Tile
{
    bool isVisible{false};
};

struct GameState
{
    bool initialized{false};
    Tile tilesWorld[GRID_WORLD.x][GRID_WORLD.y];
    GameState()
    {
        for (int y = 0; y < GRID_WORLD.x; y++)
        {
            for (int x = 0; x < GRID_WORLD.y; x++)
            {
                tilesWorld[x][y] = Tile{};
            }
        }
    }
};

extern GameState *gameState;
static ParticleSystem particle;

void init();
void simulate();
void step();
void render();

void Update(float dt);