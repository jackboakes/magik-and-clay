#include "renderer/backend/d3d11_backend.h"

#define HANDMADE_MATH_IMPLEMENTATION
#include "HandmadeMath.h"


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
    std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureStorage;
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
        RECT clientRect {};
        GetClientRect(window.handle, &clientRect);
        UINT width = clientRect.right - clientRect.left;
        UINT height = clientRect.bottom - clientRect.top;

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

    void EndFrame(const std::vector<SpriteInstance>& spriteQueue, HMM_Mat4 viewProjection)
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
        D3D11::context->PSSetSamplers(0, 1, D3D11::pointSampler.GetAddressOf());

        D3D11::context->OMSetBlendState(D3D11::blendState.Get(), 0, 0xFFFFFFFF);

        // draw sprites
        for (const auto& sprite : spriteQueue)
        {
            HMM_Mat4 model {
                HMM_MulM4(HMM_Translate(HMM_V3(sprite.destination.x, sprite.destination.y, 0.0f)),
                HMM_Scale(HMM_V3(sprite.destination.width, sprite.destination.height, 0.0f)))
            };

            Constants constants;
            constants.model = model;
            constants.viewProjection = viewProjection;

            {
                D3D11_MAPPED_SUBRESOURCE constantBufferMSR;
                context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &constantBufferMSR);
                memcpy(constantBufferMSR.pData, &constants, sizeof(Constants));
                context->Unmap(constantBuffer.Get(), 0);
            }

            auto& textureSRV { textureStorage[sprite.texture.handle] };

            context->PSSetShaderResources(0, 1, textureSRV.GetAddressOf());
            context->DrawIndexed(6, 0, 0);
        }

        // swap buffer
        window.swapChain->Present(1, 0);
    }

    Texture CreateTexture(unsigned char* textureData, int width, int height)
    {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_IMMUTABLE;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA subData{};
        subData.pSysMem = textureData;
        subData.SysMemPitch = width * 4;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
        device->CreateTexture2D(&desc, &subData, &texture);

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
        device->CreateShaderResourceView(texture.Get(), nullptr, &srv);

        uint32_t handle = static_cast<uint32_t>(textureStorage.size());
        textureStorage.push_back(srv);

        return Texture{handle};
    }
}