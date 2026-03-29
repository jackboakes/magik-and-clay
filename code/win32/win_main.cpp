#define UNICODE

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <string>

#include "renderer/renderer_d3d11.h"

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


        D3D11::BeginFrame();
        {
            D3D11::context->OMSetBlendState(D3D11::blendState.Get(), 0, 0xFFFFFFFF);
            // setup input assembly
            UINT stride { sizeof(Vertex) };
            UINT offset { 0 };
            D3D11::context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            D3D11::context->IASetInputLayout(D3D11::inputLayout.Get());
            D3D11::context->IASetVertexBuffers(0, 1, D3D11::vertexBuffer.GetAddressOf(), &stride, &offset);
            D3D11::context->IASetIndexBuffer(D3D11::indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

            // setup shaders
            D3D11::context->VSSetShader(D3D11::vertexShader.Get(), nullptr, 0);
            D3D11::context->VSSetConstantBuffers(0, 1, D3D11::constantBuffer.GetAddressOf());

            D3D11::context->PSSetShader(D3D11::pixelShader.Get(), nullptr, 0);
            D3D11::context->PSSetShaderResources(0, 1, D3D11::textureSRV.GetAddressOf());
            D3D11::context->PSSetSamplers(0, 1, D3D11::pointSampler.GetAddressOf());

            // draw
            D3D11::context->DrawIndexed(ARRAYSIZE(indices), 0, 0);
        }
        D3D11::EndFrame();
    }

    return 0;
}
