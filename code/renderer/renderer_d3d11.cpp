#include "renderer/renderer_d3d11.h"

#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"

// TODO:: later on maybe this should not be in this file at this layer level?
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


namespace D3D11
{
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    Microsoft::WRL::ComPtr<ID3D11Debug> debug;
    Microsoft::WRL::ComPtr<IDXGIFactory2> factory;

    Window window;

    // Renderer
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> pointSampler;
    Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

    void Init()
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
                &device,
                &deviceFeatureLevel,
                &context
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
            HRESULT debugResult { device.As(&debug) };

            if (debugResult != S_OK)
            {
                error = L"D3D11: Failed to get debug interface";
            }
        }

        // factory
        if (error.empty())
        {
            HRESULT factoryResult { CreateDXGIFactory1(IID_PPV_ARGS(&factory)) };

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

        // blend state
        {
            D3D11_BLEND_DESC blendDesc {};
            blendDesc.RenderTarget[0].BlendEnable = true;
            blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
            blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
            blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            device->CreateBlendState(&blendDesc, &blendState);
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
                device->CreateVertexShader(vertexShaderCSO->GetBufferPointer(), vertexShaderCSO->GetBufferSize(), 0, &vertexShader);
                device->CreatePixelShader(pixelShaderCSO->GetBufferPointer(), pixelShaderCSO->GetBufferSize(), 0, &pixelShader);

                D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
                {
                    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
                };
                device->CreateInputLayout(layoutDesc, ARRAYSIZE(layoutDesc), vertexShaderCSO->GetBufferPointer(), vertexShaderCSO->GetBufferSize(), &inputLayout);

                D3D11_BUFFER_DESC vertexBufferDesc {};
                vertexBufferDesc.ByteWidth = sizeof(vertices);
                vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
                vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

                D3D11_SUBRESOURCE_DATA vertexBufferSRD { vertices };
                device->CreateBuffer(&vertexBufferDesc, &vertexBufferSRD, &vertexBuffer);
            }

            // index buffer
            if (error.empty())
            {
                D3D11_BUFFER_DESC indexBufferDesc {};
                indexBufferDesc.ByteWidth = sizeof(indices);
                indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
                indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

                D3D11_SUBRESOURCE_DATA indexBufferSRD { indices };
                device->CreateBuffer(&indexBufferDesc, &indexBufferSRD, &indexBuffer);
            }
        }
        // Constant buffer
        {
            D3D11_BUFFER_DESC constantBufferDesc {};
            constantBufferDesc.ByteWidth = sizeof(Constants);
            constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            device->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);
        }

        // Texture
        {
            int width;
            int height;
            unsigned char* textureData { stbi_load("../data/textures/golem.png", &width, &height, nullptr, 4) };

            if (!textureData)
            {
                error = L"STB: Failed to load texture";
            }

            if (error.empty())
            {
                // create texture
                D3D11_TEXTURE2D_DESC textureDesc {};
                textureDesc.Width = width;
                textureDesc.Height = height;
                textureDesc.MipLevels = 1;
                textureDesc.ArraySize = 1;
                textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                textureDesc.SampleDesc.Count = 1;
                textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
                textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

                ID3D11Texture2D* texture;
                D3D11_SUBRESOURCE_DATA textureSRD {};
                textureSRD.pSysMem = textureData;
                textureSRD.SysMemPitch = width * 4;
                device->CreateTexture2D(&textureDesc, &textureSRD, &texture);

                // create texture view
                D3D11_SHADER_RESOURCE_VIEW_DESC textureSRVDesc;
                textureSRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                textureSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                textureSRVDesc.Texture2D.MipLevels = textureDesc.MipLevels;
                textureSRVDesc.Texture2D.MostDetailedMip = 0;
                device->CreateShaderResourceView(texture, &textureSRVDesc, textureSRV.GetAddressOf());
                texture->Release();

                stbi_image_free(textureData);
            }
        }

        // point sampler
        {
            D3D11_SAMPLER_DESC pointSamplerDesc {};
            pointSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            pointSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            pointSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            pointSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            pointSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

            device->CreateSamplerState(&pointSamplerDesc, &pointSampler);
        }

        if (!error.empty())
        {
            MessageBoxW(nullptr, error.c_str(), L"Error", MB_ICONERROR | MB_OK);
            ExitProcess(1);
        }
    }

     void WindowEquip(HWND handle)
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

            HRESULT swapChainResult { factory->CreateSwapChainForHwnd(
                device.Get(),
                handle,
                &swapChainDesc,
                &swapChainFullscreenDesc,
                0,
                &window.swapChain
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

    void BeginFrame()
    {
        Constants mvp;
        mvp.model = HMM_MulM4(
            HMM_Translate(HMM_V3(0.0f, 0.0f, 0.0f)),
            HMM_Scale(HMM_V3(32.0f, 32.0f, 1.0f))
        );

        RECT clientRect {};
        GetClientRect(window.handle, &clientRect);
        UINT width = clientRect.right - clientRect.left;
        UINT height = clientRect.bottom - clientRect.top;

        constexpr float VIRTUAL_HEIGHT = 360.0f;
        float aspect = (float)width / (float)height;
        float virtualWidth = VIRTUAL_HEIGHT * aspect;

        HMM_Mat4 view { HMM_M4D(1.0f) };
        HMM_Mat4 projection { HMM_Orthographic_RH_ZO(
        0.0f, virtualWidth,   // right edge grows on wider screens
        VIRTUAL_HEIGHT, 0.0f, // top/bottom always 360 units
        NEAR_PLANE, FAR_PLANE
        ) };

        HMM_Mat4 viewProjection { HMM_MulM4(projection, view) };
        mvp.viewProjection = viewProjection;

        bool resolutionChanged { (window.lastWidth != width ||
            window.lastHeight != height) };

        window.lastWidth = width;
        window.lastHeight = height;

        // resize swapchain and framebuffer
        if (resolutionChanged)
        {
            context->OMSetRenderTargets(0, 0, 0);
            window.view.Reset();
            window.swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

            ID3D11Texture2D* framebuffer = 0;
            window.swapChain->GetBuffer(0, IID_PPV_ARGS(&framebuffer));
            device->CreateRenderTargetView(framebuffer, 0, &window.view);
            framebuffer->Release();
        }

        {
            D3D11_MAPPED_SUBRESOURCE constantBufferMSR;
            context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &constantBufferMSR);
            memcpy(constantBufferMSR.pData, &mvp, sizeof(Constants));
            context->Unmap(constantBuffer.Get(), 0);
        }

        constexpr float clearColour[] = { 1.0f, 0.0f, 1.0f, 1.0f };
        context->ClearRenderTargetView(window.view.Get(), clearColour);

        // Update viewport
        {
            UINT width = window.lastWidth;
            UINT height = window.lastHeight;
            D3D11_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };

            context->OMSetRenderTargets(1, window.view.GetAddressOf(),
                nullptr);
            context->RSSetViewports(1, &viewport);
        }
    }

    void EndFrame()
    {
        window.swapChain->Present(1, 0);
    }
}