#include <game.h>

float fixedTime = 0.0f;

// start game 
void init()
{
}

// user input
void simulate()
{
}

// physics
void step()
{
}

void render()
{
    vec2 mouse = input->mousePosScreen;
    DrawSprite(SPRITE_REDBALL, mouse - vec2(4.0f), vec2(8.0f));
}

// Update Entry
void Update(float dt)
{
    if (!gameState->initialized)
    {
        init();
        gameState->initialized = true;
    }

    fixedTime += dt;

    while (fixedTime >= FIXED_DELTATIME)
    {
        simulate();
        step();
        particle.Update(fixedTime);
        fixedTime -= FIXED_DELTATIME;
    }

    render();
}
