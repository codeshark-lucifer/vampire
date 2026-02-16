#include <game.h>

float fixedTime = 0.0f;

Tile *getTile(int x, int y)
{
    Tile *tile = nullptr;
    if (x >= 0 && x < GRID_WORLD.x && y >= 0 && y < GRID_WORLD.y)
        tile = &gameState->tilesWorld[x][y];

    return tile;
}

vec2 getTilePos(int x, int y)
{
    float tileWidth  = (float)TILESIZE;
    float tileHeight = TILESIZE * 0.5f;

    float halfW = tileWidth * 0.5f;
    float halfH = tileHeight * 0.5f;

    // Isometric transform
    float isoX = (x - y) * halfW;
    float isoY = (x + y) * halfH;

    // --- Compute true bounds ---
    float isoMinX = (0 - (GRID_WORLD.y - 1)) * halfW;
    float isoMaxX = ((GRID_WORLD.x - 1) - 0) * halfW;

    float isoMinY = 0;
    float isoMaxY = ((GRID_WORLD.x - 1) + (GRID_WORLD.y - 1)) * halfH;

    float gridWidth  = isoMaxX - isoMinX;
    float gridHeight = isoMaxY - isoMinY;

    // --- Center on screen ---
    float offsetX = (input->screenSize.x - gridWidth) * 0.5f - isoMinX;
    float offsetY = (input->screenSize.y - gridHeight) * 0.5f;

    return vec2(isoX + offsetX, isoY + offsetY);
}


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

// draw
void render()
{
    for (int y = 0; y < GRID_WORLD.x; y++)
    {
        for (int x = 0; x < GRID_WORLD.y; x++)
        {
            Tile *tile = getTile(x, y);
            if (!tile)
                continue;
            Sprite sprite = getSprite(SPRITE_TILETEMP);
            Transform trans = {};
            trans.color = vec4(1.0f);
            trans.ioffset = sprite.offset;
            trans.isize = sprite.size;
            trans.pos = getTilePos(x, y);
            trans.size = vec2(TILESIZE);
            DrawQuad(trans);
        }
    }

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
