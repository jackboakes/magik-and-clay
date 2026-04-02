#define UNICODE

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <string>

#include "renderer/backend/d3d11_backend.h"
#include "renderer/renderer.h"

static bool running;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result { 0 };
    switch (uMsg)
    {
        case WM_CLOSE:
        {
            running = false;
            result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }
        break;
        case WM_DESTROY:
        {
            running = false;
            result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }
        break;
        default:
        {
            result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
        }
        break;
    }

    return result;
}

namespace W32
{
    // TODO:: Pass in a rect type?
    static HWND WindowCreate(int width, int height, std::wstring_view title)
    {
        HINSTANCE hInstance = GetModuleHandleW(0);

        // register window class
        {
            WNDCLASSW windowClass {};
            windowClass.lpfnWndProc = WindowProc;
            windowClass.hInstance = hInstance;
            windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
            windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
            windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
            windowClass.lpszClassName = L"graphicalWindow";
            RegisterClassW(&windowClass);
        }

        // create window
        HWND handle = 0;
        {
            // Ensure client area is exactly width x height
            RECT windowRect { 0, 0, width, height };
            AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

            handle = CreateWindowEx(
                0, L"graphicalWindow", title.data(), WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT,
                windowRect.right - windowRect.left,
                windowRect.bottom - windowRect.top,
                nullptr, nullptr, hInstance, nullptr
            );
        }

        return handle;
    }

    // TODO:: add microsecond timer with QueryPerformanceFrequency and QueryPerformanceCounter
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    D3D11::window.handle = W32::WindowCreate(1280, 720, L"Farming Sim Prototype");
    if (!D3D11::window.handle)
    {
        MessageBoxW(nullptr, L"WIN32: Failed to create window", L"Error", MB_ICONERROR | MB_OK);
        ExitProcess(1);
    }

    D3D11::Init();
    D3D11::WindowEquip(D3D11::window.handle);

    ShowWindow(D3D11::window.handle, SW_SHOW);

    running = true;

    Texture golem { Renderer::LoadTexture("../data/textures/golem.png") };
    Texture grass { Renderer::LoadTexture("../data/textures/grass.png") };

    RectF32 golemDest;
    golemDest.height = 32;
    golemDest.width = 32;
    golemDest.x = 64;
    golemDest.y = 64;



    while (running)
    {
        MSG message;
        while (PeekMessageW(&message, 0, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                running = false;
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
