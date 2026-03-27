#define UNICODE

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <string>

#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"

#define NEAR_PLANE -1.0f
#define FAR_PLANE 1.0f

struct Constants
{
    HMM_Mat4 model;
    HMM_Mat4 viewProjection;
};

struct Vertex
{
    float position[3];
};

constexpr Vertex vertices[] =
{
    // t1
    {{-0.5f, -0.5f, 0.0f}, }, // left  - green
    {{ -0.5f, 0.5f, 0.0f}, }, // top left - red
    {{0.5f, 0.5f, 0.0f}, }, // top right - yellow 
    
    // t2
    {{ 0.5f, -0.5f, 0.0f }, }, // right - blue

};

constexpr uint16_t indices[] =
{
    0, 1, 2, // t1
    0, 2, 3 // t2
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
        Microsoft::WRL::ComPtr<ID3D11Texture2D> depthBuffer;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthView;

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
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

    static void Init()
    {
        std::wstring error;

        // create device
        if (error.empty())
        {
            D3D_FEATURE_LEVEL deviceFeatureLevel;
            UINT flags = 0;
#if defined(DEBUG) || defined(_DEBUG)
             flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif


            HRESULT deviceResult { D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0,
                flags, 0, 0,
                D3D11_SDK_VERSION,
                &D3D11::device,
                &deviceFeatureLevel,
                &D3D11::context
            ) };

            if (deviceResult != S_OK)
            {
                error = L"D3D11: Failed to create device";
            }

            // TODO:: Not sure how to handle this situation or collect this error
            if (deviceFeatureLevel != D3D_FEATURE_LEVEL_11_0)
            {
                error = L"D3D11: Direct3D Feature Level 11 unsupported";
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
                {
                    error = L"D3D11: Failed to compile pixel shader";
                }
            }

            if (error.empty())
            {
                D3D11::device->CreateVertexShader(vertexShaderCSO->GetBufferPointer(), vertexShaderCSO->GetBufferSize(), 0, &D3D11::vertexShader);
                D3D11::device->CreatePixelShader(pixelShaderCSO->GetBufferPointer(), pixelShaderCSO->GetBufferSize(), 0, &D3D11::pixelShader);

                D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
                {
                    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
        // Constant buffer
        {
            D3D11_BUFFER_DESC constantBufferDesc {};
            constantBufferDesc.ByteWidth = sizeof(Constants);
            constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            D3D11::device->CreateBuffer(&constantBufferDesc, nullptr, &D3D11::constantBuffer);
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
        Constants mvp;
        mvp.model = HMM_MulM4(
            HMM_Translate(HMM_V3(0.0f, 300.0f, 0.0f)),
            HMM_Scale(HMM_V3(100.0f, 100.0f, 1.0f))
        );

        HMM_Mat4 view { HMM_M4D(1.0f) };
        HMM_Mat4 projection { HMM_Orthographic_RH_ZO(-640.0f, 640.0f, -360.0f, 360.0f, NEAR_PLANE, FAR_PLANE) };
        HMM_Mat4 viewProjection { HMM_MulM4(projection, view) };
        mvp.viewProjection = viewProjection;
        
        RECT clientRect {};
        GetClientRect(D3D11::window.handle, &clientRect);
        UINT width = clientRect.right - clientRect.left;
        UINT height = clientRect.bottom - clientRect.top;

        bool resolutionChanged { (D3D11::window.lastWidth != width ||
            D3D11::window.lastHeight != height) };

        D3D11::window.lastWidth = width;
        D3D11::window.lastHeight = height;

        // resize swapchain and framebuffer
        if(resolutionChanged)
        {
            D3D11::context->OMSetRenderTargets(0, 0, 0);
            D3D11::window.view.Reset();
            D3D11::window.swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

            ID3D11Texture2D* framebuffer = 0;
            D3D11::window.swapChain->GetBuffer(0, IID_PPV_ARGS(&framebuffer));
            D3D11::device->CreateRenderTargetView(framebuffer, 0, &D3D11::window.view);
            framebuffer->Release();
        }

        if (resolutionChanged)
        {
            // reset old resources
            D3D11::window.depthBuffer.Reset();
            D3D11::window.depthView.Reset();


            // create depth buffer target
            {
                D3D11_TEXTURE2D_DESC depthDesc {};
                depthDesc.Width = width;
                depthDesc.Height = height;
                depthDesc.MipLevels = 1;
                depthDesc.ArraySize = 1;
                depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
                depthDesc.SampleDesc.Count = 1;
                depthDesc.SampleDesc.Quality = 0;
                depthDesc.Usage = D3D11_USAGE_DEFAULT;
                depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
                depthDesc.CPUAccessFlags = 0;
                depthDesc.MiscFlags = 0;

                D3D11::device->CreateTexture2D(&depthDesc, nullptr, &D3D11::window.depthBuffer);
                D3D11::device->CreateDepthStencilView(D3D11::window.depthBuffer.Get(), nullptr, &D3D11::window.depthView);
            }
        }

        {
            D3D11_MAPPED_SUBRESOURCE constantBufferMSR;
            context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &constantBufferMSR);
            memcpy(constantBufferMSR.pData, &mvp, sizeof(Constants));
            D3D11::context->Unmap(D3D11::constantBuffer.Get(), 0);
        }

        constexpr float clearColour[] = { 1.0f, 0.0f, 1.0f, 1.0f };
        D3D11::context->ClearRenderTargetView(D3D11::window.view.Get(), clearColour);

        // Update viewport
        {
            UINT width = D3D11::window.lastWidth;
            UINT height = D3D11::window.lastHeight;
            D3D11_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };

            D3D11::context->OMSetRenderTargets(1, D3D11::window.view.GetAddressOf(),
                D3D11::window.depthView.Get());
            D3D11::context->RSSetViewports(1, &viewport);
            D3D11::context->ClearDepthStencilView(D3D11::window.depthView.Get(),
                D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
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

            // draw
            D3D11::context->DrawIndexed(ARRAYSIZE(indices), 0, 0);
        }
        D3D11::EndFrame();
    }

    return 0;
}
