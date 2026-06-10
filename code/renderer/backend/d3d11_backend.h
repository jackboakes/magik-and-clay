#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <string>

#include "HandmadeMath.h"
#include "renderer/renderer.h"

#include <vector>

static constexpr F32 g_NearPlane { 0.0f };
static constexpr F32 g_FarPlane { 100.0f };

struct InstanceData
{
    HMM_Mat4 model;
    HMM_Vec2 sourcePos;
    HMM_Vec2 textureSize;
    HMM_Vec2 spriteSize;
};

struct Constants
{
    //HMM_Mat4 model;
    HMM_Mat4 viewProjection;
    //HMM_Vec2 sourcePosition;
    //HMM_Vec2 textureSize;
    //HMM_Vec2 spriteSize;
};

struct Vertex
{
    F32 position[3];
    F32 uv[2];
};

// Pivot point is top left like raylib
// TODO:: at the moment I'm thinking that there should be two quad functions or a way to change the pivot
// tiles i'd prefer to be at top left pivot but certain objects i'd prefer the pivot to be in the centre.
constexpr Vertex vertices[] =
{
    {0.0f, 0.0f, 0.0f,  0.0f, 0.0f},  // top left
    {1.0f, 0.0f, 0.0f,  1.0f, 0.0f},  // top right
    {1.0f, 1.0f, 0.0f,  1.0f, 1.0f},  // bottom right
    {0.0f, 1.0f, 0.0f,  0.0f, 1.0f},  // bottom left
};

constexpr U16 indices[] =
{
    0, 1, 2, // t1
    0, 2, 3 // t2
};

// TODO:: W32_D3D11 globals maybe make the globals a struct like the window?
namespace D3D11
{

    extern U32 internalDrawCallCount;
    extern U32 drawCallCount;

    extern Microsoft::WRL::ComPtr<ID3D11Device> device;
    extern Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    extern Microsoft::WRL::ComPtr<ID3D11Debug> debug;
    extern Microsoft::WRL::ComPtr<IDXGIFactory2> factory;


    // TODO:: We can pull out the win32 coupling and pass this d3d11 window equpping
    // related data to a struct w32window via a template type.
    struct Window
    {
        HWND handle;
        Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> view;

        // TODO:: Change this into a int32_t Vec2 lastResolution variable
        U32 lastWidth { 0 };
        U32 lastHeight { 0 };
    };

    extern Window window;

    // Renderer
    extern Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    extern Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    extern Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    extern Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    extern Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    extern Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
    extern std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureStorage;
    extern Microsoft::WRL::ComPtr<ID3D11SamplerState> pointSampler;
    extern Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

    void Init();
    void WindowEquip(HWND handle);
    void BeginFrame();
    void SubmitFrame(std::vector<RenderPass>& passes);
    void EndFrame();

    U32 CreateTexture(U8* textureData, S32 width, S32 height);
}