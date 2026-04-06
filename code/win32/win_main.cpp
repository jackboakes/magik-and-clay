#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "win_core.h"
#include "renderer/renderer.h"


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    Renderer::WindowCreate(1280, 720, L"Farming Sim Prototype");


    W32::running = true;

    Texture golem { Renderer::LoadTexture("../data/textures/golem.png") };
    Texture grass { Renderer::LoadTexture("../data/textures/grass.png") };

    RectF32 golemDest;
    golemDest.height = 32;
    golemDest.width = 32;
    golemDest.x = 64;
    golemDest.y = 64;

    while (W32::running)
    {
        MSG message;
        while (PeekMessageW(&message, 0, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                W32::running = false;
            }
            TranslateMessage(&message);
            DispatchMessage(&message);
        }


        Renderer::BeginFrame();

            for (int i { 0 }; i < 1280; i += 32)
            {
                for (int j = { 0 }; j < 360; j += 32)
                {
                    RectF32 grassDest;
                    grassDest.height = 32;
                    grassDest.width = 32;
                    grassDest.x = i;
                    grassDest.y = j;
                    Renderer::DrawSprite(grass, grassDest);
                }
            }

            Renderer::DrawSprite(golem, golemDest);

        Renderer::EndFrame();

    }

    return 0;
}
