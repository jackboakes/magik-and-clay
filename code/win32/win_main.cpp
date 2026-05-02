/* TODO:: LIST
    - zoom to mouse pos when scrolling
    - win32 specific timer
    - vsync on/off toggle | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
    - borderless windowed mode
    - logging system
    - manage d3d11 renderer object lifetimes properly when exiting application
    - sprite animations
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>
#include <cstdint>
#include <string>
#include <algorithm>

#include "win_core.h"
#include "renderer/renderer.h"
#include "input/input.h"
#include "camera/camera.h"

#include "renderer/backend/d3d11_backend.h"

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

    double fps { 0 };
    QueryPerformanceFrequency(&frequency);

    W32::running = true;

    Texture golem { Renderer::LoadTexture("../data/textures/idle_golem.png") };
    Texture grass { Renderer::LoadTexture("../data/textures/grass.png") };
    Font font1 { Renderer::LoadFont("../data/font/romulus.ttf", 16.0f) };
    Font font2 { Renderer::LoadFont("../data/font/tiny5.ttf", 8.0f) };

    RectF32 golemDest;
    golemDest.height = 16;
    golemDest.width = 16;
    golemDest.x = 64;
    golemDest.y = 64;

    Camera camera;
    camera.position = { 0.0f, 0.0f };
    camera.zoom = 1.0f;

    std::vector<RectF32> grassDestinations;

    
    for (int i { 0 }; i < 640; i += 16)
    {
        for (int j = { 0 }; j < 360; j += 16)
        {
            RectF32 grassDest;
            grassDest.height = 16;
            grassDest.width = 16;
            grassDest.x = i;
            grassDest.y = j;
            grassDestinations.push_back(grassDest);
        }
    }

    while (W32::running)
    {
        uint64_t startTimeMS { TimeMicroseconds() };

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

        Input::ProcessEvents();

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

        float scrollWheelDelta { Input::GetScrollDelta() };
        if (scrollWheelDelta != 0.0f)
        {
            // TODO:: Figure out what amount of camera steps I want and how far each step will be,
            // linear steps should be the best for this type of game.
            camera.zoom += scrollWheelDelta / 10.0f;
            camera.zoom = std::clamp(camera.zoom, 0.5f, 1.5f);
        }
        HMM_Vec2 mousePos { Input::GetMousePosition() };

        // TODO:: abstract over the clientrect from window
        RectF32 clientRect { W32::ClientRectFromWindow(D3D11::window.handle) };
        constexpr float virtualHeight { 360.0f };
        float aspectRatio { clientRect.width / clientRect.height };
        float virtualWidth { virtualHeight * aspectRatio };

        Renderer::BeginFrame(virtualWidth, virtualHeight);

            Renderer::BeginModeWorldSpace(camera);

                for (const auto& grassDest : grassDestinations)
                {
                    Renderer::DrawSprite(grass, grassDest);
                }

                uint64_t currentTimeMS = TimeMicroseconds() / 1000;
                int frameIndex = (currentTimeMS / 500) % 2;
                float srcX = frameIndex == 0 ? 2.0f : 20.0f;
                RectF32 golemSrc = { srcX, 2.0f, 16.0f, static_cast<float>(golem.height) - 4.0f };

                Renderer::DrawSprite(golem, golemDest, golemSrc);

            Renderer::EndMode();

            Renderer::BeginModeScreenSpace();

                Renderer::DrawSprite(golem, {0,328, 32, 32}, golemSrc);

                Renderer::DrawText(font1, "FPS: " + std::to_string(static_cast<int>(fps)), 5, 5, font1.size);
                Renderer::DrawText(font2, "Sprite Batch Count: " + std::to_string(D3D11::drawCallCount), 5, 15, font2.size);

            Renderer::EndMode();

        Renderer::EndFrame();

        uint64_t frameTimeMicroseconds { TimeMicroseconds() - startTimeMS };
        double frameTimeMilliseconds { static_cast<double>(frameTimeMicroseconds) / 1000.0 };

        static double timeAccumulator = 0.0;
        static int frameCount = 0;

        double frameTimeSeconds = static_cast<double>(frameTimeMicroseconds) / 1'000'000.0;

        timeAccumulator += frameTimeSeconds;
        frameCount++;

        if (timeAccumulator >= 1.0) // update 4 times per second
        {
            fps = frameCount / timeAccumulator;

            std::wstring title = L"Farming Prototype | FPS: " + std::to_wstring(static_cast<int>(fps));
            SetWindowTextW(D3D11::window.handle, title.c_str());

            // reset
            timeAccumulator = 0.0;
            frameCount = 0;
        }
    }

    return 0;
}
