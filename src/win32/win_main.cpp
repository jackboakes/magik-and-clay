#define UNICODE

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>

// TODO:: W32_D3d11 globals maybe make the globals a struct like the window?
namespace D3D11
{
    //static HMODULE dxgiModule;
    //static HMODULE d3d11Module;
    //static HMODULE d3compilerModule;

    static Microsoft::WRL::ComPtr<ID3D11Device> device;
    static Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    static Microsoft::WRL::ComPtr<ID3D11Debug> debug;
    static Microsoft::WRL::ComPtr<IDXGIFactory2> factory;


    // TODO:: We can pull out the win32 coupling and pass this d3d11 window equpping
    // related data to a struct w32window via a template type.
    struct Window
    {
        HWND handle;
        Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> buffer;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> view;
        D3D11_VIEWPORT viewport;
    };

    static Window window;
}

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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    const std::wstring windowClassName { L"D3D11WindowClass" };

    std::wstring error;

    {
        // create device
        if (error.empty())
        {
            constexpr D3D_FEATURE_LEVEL deviceFeatureLevel { D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0 };
            UINT flags { D3D11_CREATE_DEVICE_DEBUG };

            HRESULT deviceResult { D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0,
                flags, &deviceFeatureLevel, 1,
                D3D11_SDK_VERSION,
                &D3D11::device,
                0,
                &D3D11::context
            ) };

            if (deviceResult != S_OK)
            {
                error = L"D3D11: Failed to create device";
            }
        }

        // debug
        if (error.empty())
        {
            HRESULT debugResult { D3D11::device.As(&D3D11::debug) };

            if (debugResult != S_OK)
            {
                error = L"D3D11: Failed to get debug interface";
            }
        }

        // factory
        {
            if (error.empty())
            {
                HRESULT factoryResult { CreateDXGIFactory1(IID_PPV_ARGS(&D3D11::factory)) };

                if (factoryResult != S_OK)
                {
                    error = L"D3D11: Unable to create DXGIFactory";
                }
            }
        }
    }

    {
        // register window class
        WNDCLASSW windowClass {};
        windowClass.lpfnWndProc = WindowProc;
        windowClass.hInstance = hInstance;
        windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        windowClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        windowClass.lpszClassName = windowClassName.c_str();

        if (error.empty())
        {
            if (!RegisterClassW(&windowClass))
            {
                error = L"WIN32: Failed to create window";
            }
        }

        // create window
        if (error.empty())
        {
            // Ensure client area is exactly 1280x720
            RECT windowRect { 0, 0, 1280, 720 };
            AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

            D3D11::window.handle = CreateWindowEx(
                0, windowClassName.c_str(), L"Farming Sim Prototype", WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT, 
                windowRect.right - windowRect.left,
                windowRect.bottom - windowRect.top,
                nullptr, nullptr, hInstance, nullptr
            );

            if (!D3D11::window.handle)
            {
                error = L"WIN32: Failed to create window";
            }
        }
    }

    {
        //swap chain
        if (error.empty())
        {
            // Get the drawable area (the window area excluding the title bar and border)
            RECT clientRect {};
            GetClientRect(D3D11::window.handle, &clientRect);

            DXGI_SWAP_CHAIN_DESC1 swapChainDesc {};
            swapChainDesc.Width = clientRect.right - clientRect.left;
            swapChainDesc.Height = clientRect.bottom - clientRect.top;
            swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapChainDesc.SampleDesc.Count = 1;
            swapChainDesc.SampleDesc.Quality = 0;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount = 2;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            // TODO:: change scaling to none when implementing virtual resolution
            swapChainDesc.Scaling = DXGI_SCALING_STRETCH;

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDesc {};
            swapChainFullscreenDesc.Windowed = true;

            HRESULT swapChainResult { D3D11::factory->CreateSwapChainForHwnd(
                D3D11::device.Get(),
                D3D11::window.handle,
                &swapChainDesc,
                &swapChainFullscreenDesc,
                0,
                &D3D11::window.swapChain
            ) };

            if (swapChainResult != S_OK)
            {
                error = L"D3D11: Failed to create swap chain";
            }
        }

        // texture buffer
        if (error.empty())
        {
            HRESULT bufferResult { D3D11::window.swapChain->GetBuffer(0, IID_PPV_ARGS(&D3D11::window.buffer)) };

            if (bufferResult != S_OK)
            {
                error = L"D3D11: Failed to get buffer";
            }
        }

        // view
        if (error.empty())
        {
            HRESULT viewResult { D3D11::device->CreateRenderTargetView(D3D11::window.buffer.Get(), 0, &D3D11::window.view) };

            if (viewResult != S_OK)
            {
                error = L"D3D11: Failed to create render target view";
            }
        }
    }

    // viewport
    {

        RECT clientRect {};
        GetClientRect(D3D11::window.handle, &clientRect);

        D3D11::window.viewport.Width = static_cast<float>(clientRect.right - clientRect.left);
        D3D11::window.viewport.Height = static_cast<float>(clientRect.bottom - clientRect.top);
        D3D11::window.viewport.MinDepth = 0.0f;
        D3D11::window.viewport.MaxDepth = 1.0f;
    }

    if (!error.empty())
    {
        MessageBoxW(nullptr, error.c_str(), L"Error", MB_ICONERROR | MB_OK);
        return 1;
    }

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

        constexpr float clearColour[] = { 1.0f, 0.0f, 1.0f, 1.0f };
        D3D11::context->ClearRenderTargetView(D3D11::window.view.Get(), clearColour);
        D3D11::context->RSSetViewports(1, &D3D11::window.viewport);
        D3D11::context->OMSetRenderTargets(1, D3D11::window.view.GetAddressOf(), 0);

        D3D11::window.swapChain->Present(1, 0);
    }

    return 0;
}
