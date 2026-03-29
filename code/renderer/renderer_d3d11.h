#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <string>

#include "HandmadeMath.h"

#define NEAR_PLANE 0.0f
#define FAR_PLANE 100.0f

struct Constants
{
    HMM_Mat4 model;
    HMM_Mat4 viewProjection;
};

struct Vertex
{
    float position[3];
    float uv[2];
};

// Pivot point is top left like raylib'
// TODO:: at the moment I'm thinking that there should be two quad functions or a way to change the pivot
// tiles i'd prefer to be at top left pivot but certain objects i'd prefer the pivot to be in the centre.
constexpr Vertex vertices[] =
{
    {0.0f, 0.0f, 0.0f,  0.0f, 0.0f},  // top left
    {1.0f, 0.0f, 0.0f,  1.0f, 0.0f},  // top right
    {1.0f, 1.0f, 0.0f,  1.0f, 1.0f},  // bottom right
    {0.0f, 1.0f, 0.0f,  0.0f, 1.0f},  // bottom left
};

constexpr uint16_t indices[] =
{
    0, 1, 2, // t1
    0, 2, 3 // t2
};

// TODO:: W32_D3D11 globals maybe make the globals a struct like the window?
namespace D3D11
{
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

        // TODO:: Change this into a Vec2 lastResolution variable
        UINT lastWidth { 0 };
        UINT lastHeight { 0 };
    };

    extern Window window;

    // Renderer
    extern Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    extern Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    extern Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    extern Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    extern Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    extern Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
    extern Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
    extern Microsoft::WRL::ComPtr<ID3D11SamplerState> pointSampler;
    extern Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;

    void Init();
    void WindowEquip(HWND handle);
    void BeginFrame();
    void EndFrame();
}