#define UNICODE

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <string>

struct Vertex
{
    float position[3];
    float colour[3];
};

constexpr Vertex vertices[] =
{
    // t1
    {{ -0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // top left - red
    {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // right - blue
    {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // left  - green
    // t2
    {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 1.0f}} // top right - cyan

};

constexpr uint16_t indices[] =
{
    0, 1, 2, // t1
    0, 3, 1 // t2
};

// TODO:: W32_D3D11 globals maybe make the globals a struct like the window?
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
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> view;

        // TODO:: Change this into a Vec2 lastResolution variable
        UINT lastWidth;
        UINT lastHeight;
    };

    static Window window;


    // Renderer
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

    static void Init()
    {
        std::wstring error;

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
        if (error.empty())
        {
            HRESULT factoryResult { CreateDXGIFactory1(IID_PPV_ARGS(&D3D11::factory)) };

            if (factoryResult != S_OK)
            {
                error = L"D3D11: Unable to create DXGIFactory";
            }
        }

        if (!error.empty())
        {
            MessageBoxW(nullptr, error.c_str(), L"Error", MB_ICONERROR | MB_OK);
            ExitProcess(1);
        }

        // Shader
        {
            Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderCSO;
            Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderCSO;

            if (error.empty())
            {
                Microsoft::WRL::ComPtr<ID3DBlob> vsError;
                HRESULT vsResult { D3DCompileFromFile(L"../data/shaders/basicvs.hlsl", 0, 0, "VSMain", "vs_5_0", 0, 0, &vertexShaderCSO, &vsError) };

                if (vsResult != S_OK)
                    error = L"D3D11: Failed to compile vertex shader";
            }

            if (error.empty())
            {
                Microsoft::WRL::ComPtr<ID3DBlob> psError;
                HRESULT psResult { D3DCompileFromFile(L"../data/shaders/basicps.hlsl", 0, 0, "PSMain", "ps_5_0", 0, 0, &pixelShaderCSO, &psError) };

                if (psResult != S_OK)
                    error = L"D3D11: Failed to compile pixel shader";
            }

            if (error.empty())
            {
                D3D11::device->CreateVertexShader(vertexShaderCSO->GetBufferPointer(), vertexShaderCSO->GetBufferSize(), 0, &D3D11::vertexShader);
                D3D11::device->CreatePixelShader(pixelShaderCSO->GetBufferPointer(), pixelShaderCSO->GetBufferSize(), 0, &D3D11::pixelShader);

                D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
                {
                    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                };
                D3D11::device->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), vertexShaderCSO->GetBufferPointer(), vertexShaderCSO->GetBufferSize(), &D3D11::inputLayout);

                D3D11_BUFFER_DESC vertexBufferDesc {};
                vertexBufferDesc.ByteWidth = sizeof(vertices);
                vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
                vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

                D3D11_SUBRESOURCE_DATA vertexBufferSRD { vertices };
                D3D11::device->CreateBuffer(&vertexBufferDesc, &vertexBufferSRD, &D3D11::vertexBuffer);
            }

            // index buffer
            if (error.empty())
            {
                D3D11_BUFFER_DESC indexBufferDesc {};
                indexBufferDesc.ByteWidth = sizeof(indices);
                indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
                indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

                D3D11_SUBRESOURCE_DATA indexBufferSRD { indices };
                D3D11::device->CreateBuffer(&indexBufferDesc, &indexBufferSRD, &D3D11::indexBuffer);
            }
        }

        if (!error.empty())
        {
            MessageBoxW(nullptr, error.c_str(), L"Error", MB_ICONERROR | MB_OK);
            ExitProcess(1);
        }
    }

    static void WindowEquip(HWND handle)
    {
        std::wstring error;

        //swap chain
        {
            // Get the drawable area (the window area excluding the title bar and border)
            RECT clientRect {};
            GetClientRect(handle, &clientRect);

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
                handle,
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

        if (!error.empty())
        {
            MessageBoxW(nullptr, error.c_str(), L"Error", MB_ICONERROR | MB_OK);
            ExitProcess(1);
        }
    }

    static void BeginFrame()
    {
        RECT clientRect {};
        GetClientRect(D3D11::window.handle, &clientRect);
        UINT width = clientRect.right - clientRect.left;
        UINT height = clientRect.bottom - clientRect.top;

        bool resolutionChanged { (D3D11::window.lastWidth != width ||
            D3D11::window.lastHeight != height) };

        D3D11::window.lastWidth = width;
        D3D11::window.lastHeight = height;

        if (resolutionChanged)
        {
            D3D11::context->OMSetRenderTargets(0, 0, 0);
            D3D11::window.view.Reset();

            D3D11::window.swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

            ID3D11Texture2D* framebuffer = 0;
            D3D11::window.swapChain->GetBuffer(0, IID_PPV_ARGS(&framebuffer));
            D3D11::device->CreateRenderTargetView(framebuffer, 0, &D3D11::window.view);
            framebuffer->Release();
        }

        constexpr float clearColour[] = { 1.0f, 0.0f, 1.0f, 1.0f };
        D3D11::context->ClearRenderTargetView(D3D11::window.view.Get(), clearColour);

        // Update viewport
        {
            UINT width = D3D11::window.lastWidth;
            UINT height = D3D11::window.lastHeight;
            D3D11_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };

            D3D11::context->RSSetViewports(1, &viewport);
            D3D11::context->OMSetRenderTargets(1, D3D11::window.view.GetAddressOf(), nullptr);
        }
    }

    static void EndFrame()
    {
        D3D11::window.swapChain->Present(1, 0);
    }
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
            // setup input assembly
            UINT stride { sizeof(Vertex) };
            UINT offset { 0 };
            D3D11::context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            D3D11::context->IASetInputLayout(D3D11::inputLayout.Get());
            D3D11::context->IASetVertexBuffers(0, 1, D3D11::vertexBuffer.GetAddressOf(), &stride, &offset);
            D3D11::context->IASetIndexBuffer(D3D11::indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

            // setup shaders
            D3D11::context->VSSetShader(D3D11::vertexShader.Get(), nullptr, 0);
            D3D11::context->PSSetShader(D3D11::pixelShader.Get(), nullptr, 0);

            // draw
            D3D11::context->DrawIndexed(ARRAYSIZE(indices), 0, 0);
        }
        D3D11::EndFrame();
    }

    return 0;
}
