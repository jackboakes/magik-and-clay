/* TODO:: LIST
    - win32 specific timer
    - vsync on/off toggle | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
    - borderless windowed mode
    - logging system
    - manage d3d11 renderer object lifetimes properly when exiting application
    - sprite animations
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "win_core.h"
#include "game/game.h"



int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    W32::running = true;

    Game::Init();

    double deltaTime { 1.0 / 60.0 };
    double accumulator { 0.0 };

    double currentTime { W32::TimeSeconds() };

    while (W32::running)
    {
        double newTime { W32::TimeSeconds() };
        double frameTime { newTime - currentTime };
        currentTime = newTime;
        if (frameTime > 0.25)
        {
            frameTime = 0.25;
        } 
        accumulator += frameTime;

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

        while (accumulator >= deltaTime)
        {
            Game::UpdateAndDrawFrame(deltaTime);
            accumulator -= deltaTime;
        }

    }

    return 0;
}
