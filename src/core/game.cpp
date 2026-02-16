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
    float tileWidth = (float)TILESIZE;
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

    float gridWidth = isoMaxX - isoMinX;
    float gridHeight = isoMaxY - isoMinY;

    // --- Center on screen ---
    float offsetX = (renderData->camera.dimensions.x - gridWidth) * 0.5f - isoMinX;
    float offsetY = (renderData->camera.dimensions.y - gridHeight) * 0.5f;

    return vec2(isoX + offsetX, isoY + offsetY);
}

vec2 getMouseWorldPos()
{
    vec2 mouse = input->mousePosScreen;
    return mouse;
}
ivec2 getGridPos(vec2 screenPos)
{
    float tileWidth = (float)TILESIZE;
    float tileHeight = TILESIZE * 0.5f;

    float halfW = tileWidth * 0.5f;
    float halfH = tileHeight * 0.5f;

    // 1. Recompute the offsets exactly as you did in the forward function
    float isoMinX = (0 - (GRID_WORLD.y - 1)) * halfW;
    float isoMaxX = ((GRID_WORLD.x - 1) - 0) * halfW;
    float isoMinY = 0;
    float isoMaxY = ((GRID_WORLD.x - 1) + (GRID_WORLD.y - 1)) * halfH;

    float gridWidth = isoMaxX - isoMinX;
    float gridHeight = isoMaxY - isoMinY;

    float offsetX = (renderData->camera.dimensions.x - gridWidth) * 0.5f - isoMinX;
    float offsetY = (renderData->camera.dimensions.y - gridHeight) * 0.5f;

    // 2. Remove the offsets to get back to "Pure Isometric" space
    float isoX = screenPos.x - offsetX;
    float isoY = screenPos.y - offsetY;

    // 3. Solve the linear equations
    float x = (isoX / halfW + isoY / halfH) * 0.5f;
    float y = (isoY / halfH - isoX / halfW) * 0.5f;

    // 4. Round or Floor depending on your needs
    // We use floor to ensure we get the tile index the point is inside of.
    return ivec2((int)floor(x - 0.3f), (int)floor(y + 0.5f));
}
ivec2 getTileAtMouse()
{
    vec2 mouse = getMouseWorldPos();
    return getGridPos(mouse);
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
    // 1. Get the tile coordinate currently under the mouse
    ivec2 hoveredTile = getTileAtMouse();

    for (int y = 0; y < GRID_WORLD.y; y++)
    {
        for (int x = 0; x < GRID_WORLD.x; x++)
        {
            Tile *tile = getTile(x, y);
            if (!tile)
                continue;

            Sprite sprite = getSprite(SPRITE_TILETEMP);
            Transform trans = {};

            // 2. CHECK HOVER: If this specific tile is the one under the mouse, make it RED
            if (x == hoveredTile.x && y == hoveredTile.y)
            {
                trans.color = vec4(1.0f, 0.0f, 0.0f, 1.0f); // Red tint
            }
            else
            {
                trans.color = vec4(1.0f, 1.0f, 1.0f, 1.0f); // Normal white
            }

            trans.ioffset = sprite.offset;
            trans.isize = sprite.size;
            trans.pos = getTilePos(x, y);
            trans.size = vec2(TILESIZE);

            // This now pushes the tile with the correct color
            DrawQuad(trans);
        }
    }

    // 3. Draw the mouse cursor (The red ball)
    // Keep this white (1.0f) so it stays its original texture color
    vec2 mouseWorld = getMouseWorldPos();
    DrawSprite(SPRITE_REDBALL, mouseWorld - vec2(4.0f), vec2(8.0f), vec3(1.0f));
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
