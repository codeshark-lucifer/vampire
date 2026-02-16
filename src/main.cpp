#include <platform/win32.h>
#include <game.h>

GameState *gameState = nullptr;
int main()
{
    InitPlatform();

    gameState = BumpAlloc<GameState>(&persistentStorage);

    CreateWindowPlatform(956, 540, "atlas - engine");
    SetColorTitleBar({0.1f, 0.1f, 0.1f, 1.0f}, COLOR_WHITE);

    glInit(&persistentStorage);

    while (!ShouldClose())
    {
        Event event;
        PollEvent(&event);

        Update(deltaTime);
        std::sort(renderData->transforms.begin(), renderData->transforms.end(),
                  [](const Transform &a, const Transform &b)
                  {
                      return a.layer < b.layer;
                  });

        glRender();

        SwapBuffersWindow();
    }

    return 0;
}