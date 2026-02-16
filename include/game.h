#pragma once
#include <assets.h>
#include <interface.h>
#include <platform/win32.h>
#include <systems/particle.h>

constexpr float FIXED_DELTATIME = 1.0f / 60.0f;

constexpr int WORLD_WIDTH = 950;
constexpr int WORLD_HEIGHT = 540;
constexpr int TILESIZE = 32;
constexpr int GRID_X = (WORLD_WIDTH + TILESIZE - 1) / TILESIZE;
constexpr int GRID_Y = (WORLD_HEIGHT + TILESIZE - 1) / TILESIZE;
constexpr ivec2 WORLD_GRID = {GRID_X, GRID_Y};

struct GameState
{
    bool initialized{false};
};

extern GameState *gameState;
static ParticleSystem particle;

void init();
void simulate();
void step();
void render();

void Update(float dt);