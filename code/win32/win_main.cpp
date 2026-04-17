/* TODO LIST::
    - Scroll input
    - Sprite sheet support by adding source to sprite instance
    - win32 specific timer
    - sprite batching
    - vsync on/off toggle | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
    - borderless windowed mode
    - logging system
    - manage d3d11 renderer object lifetimes properly when exiting application
    - sprite animations
    - text rendering with stb_truetype
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>
#include <cstdint>
#include <string>

#include "win_core.h"
#include "renderer/renderer.h"
#include "input/input.h"
#include "camera/camera.h"

LARGE_INTEGER frequency;

uint64_t TimeMicroseconds() 
{
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    double timeInSeconds { static_cast<double>(currentTime.QuadPart) / static_cast<double>(frequency.QuadPart) };
    return static_cast<uint64_t>(timeInSeconds * 1000000.0);
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    Renderer::WindowCreate(1280, 720, L"Farming Sim Prototype");


    QueryPerformanceFrequency(&frequency);

    W32::running = true;

    Texture golem { Renderer::LoadTexture("../data/textures/golem.png") };
    Texture grass { Renderer::LoadTexture("../data/textures/grass.png") };

    RectF32 golemDest;
    golemDest.height = 32;
    golemDest.width = 16;
    golemDest.x = 64;
    golemDest.y = 64;

    Camera camera;
    camera.position = { 0.0f, 0.0f };
    camera.zoom = 1.0f;

    std::vector<RectF32> grassDestinations;

    
    for (int i { 0 }; i < 1280; i += 32)
    {
        for (int j = { 0 }; j < 360; j += 32)
        {
            RectF32 grassDest;
            grassDest.height = 32;
            grassDest.width = 32;
            grassDest.x = i;
            grassDest.y = j;
            grassDestinations.push_back(grassDest);
        }
    }

    while (W32::running)
    {
        uint64_t startTimeMS { TimeMicroseconds() };
        keyStatePrevious = keyStateCurrent;
        sysEventQueue.clear();

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

        for (const auto& event : sysEventQueue)
        {
            //TODO:: This is temporary as the user can skip keys if the user presses and releases on the same frame.
            if (event.type == SysEventType::KEY_PRESS)
            {
                keyStateCurrent[static_cast<size_t>(event.key)] = true;
            }
            if (event.type == SysEventType::KEY_RELEASE)
            {
                keyStateCurrent[static_cast<size_t>(event.key)] = false;
            }
        }

        if (Input::IsKeyDown(Key::W))
        {
            camera.position.Y -= 1.0f;
        }
        if (Input::IsKeyDown(Key::A))
        {
            camera.position.X -= 1.0f;
        }
        if (Input::IsKeyDown(Key::S))
        {
            camera.position.Y += 1.0f;
        }
        if (Input::IsKeyDown(Key::D))
        {
            camera.position.X += 1.0f;
        }

        Renderer::BeginFrame(camera);

            for (const auto& grassDest : grassDestinations)
            {
                    Renderer::DrawSprite(grass, grassDest);
            }

            Renderer::DrawSprite(golem, golemDest, { static_cast<float>(golem.width) * 0.5f ,0 , static_cast<float>(golem.width) * 0.5f, static_cast<float>(golem.height)});
            

        Renderer::EndFrame();

        //uint64_t frameTimeMicroseconds { TimeMicroseconds() - startTimeMS };
        //double frameTimeMilliseconds { static_cast<double>(frameTimeMicroseconds) / 1000.0 };
        //std::wstring frameTime { L"Frame time in ms: " + std::to_wstring(frameTimeMilliseconds) + L"\n"};
        //OutputDebugStringW(frameTime.data());
    }

    return 0;
}
