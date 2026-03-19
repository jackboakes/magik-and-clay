#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_3.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>

struct D3D11Context
{
	Microsoft::WRL::ComPtr<ID3D11Device>           device; // Object to create buffers, textures, samplers, shaders etc
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>    deviceContext; // issues draw and compute commands to GPU
	Microsoft::WRL::ComPtr<IDXGISwapChain1>        swapChain; // Stores render data
	Microsoft::WRL::ComPtr<IDXGIFactory2>          dxgiFactory; // Helps find the adapter to run graphics on
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTarget; 
};

static D3D11Context gfx;

static bool running;

LRESULT CALLBACK MainWindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
			result = result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
		}
		break;
	}
	// If you get here the message wasn't handled
	return result;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	// Create a win32 window class and window
	HWND windowHandle { 0 };
	std::wstring windowClassName { L"GameWindowClass" };

	{
		WNDCLASSW windowClass {};
		windowClass.lpfnWndProc = MainWindowCallback;
		windowClass.hInstance = hInstance;
		windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		windowClass.hIcon = nullptr;
		windowClass.hCursor = nullptr;
		windowClass.lpszClassName = windowClassName.c_str();

		if (!RegisterClassW(&windowClass))
		{
			MessageBoxW(nullptr, L"WIN32: Failed to register window class", L"Error", MB_ICONWARNING | MB_OK);
			return 1;
		}

		HWND hwnd { CreateWindowExW(
			0,											// Optional window styles
			windowClassName.c_str(),					// Window class
			L"Clay & Magik",							// Window title
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,			// Window style
			CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,	// Position and Size x,y,w,h
			nullptr,									// Parent window
			nullptr,									// Menu
			hInstance,									// Instance handle
			nullptr										// Additional application data
		) };

		if (!hwnd)
		{
			MessageBoxW(nullptr, L"WIN32: Failed to create window", L"Error", MB_ICONWARNING | MB_OK);
			return 1;
		}

		windowHandle = hwnd;
	}

	// directx 11 code
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&gfx.dxgiFactory))))
	{
		MessageBoxW(nullptr, L"DXGI: Unable to create DXGIFactory", L"Error", MB_ICONWARNING | MB_OK);
	}

	constexpr D3D_FEATURE_LEVEL deviceFeatureLevel { D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0 };
	if (FAILED(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		&deviceFeatureLevel,
		1,
		D3D11_SDK_VERSION,
		&gfx.device,
		nullptr,
		&gfx.deviceContext)))
	{
		MessageBoxW(nullptr, L"D3D11: Failed to create device and device Context", L"Error", MB_ICONWARNING | MB_OK);
	}

	// get win32 drawble area (excludes bars and border)
	RECT clientRect {};
	GetClientRect(windowHandle, &clientRect);

	// Swapchain resources
	DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor {};
	swapChainDescriptor.Width = clientRect.right - clientRect.left;
	swapChainDescriptor.Height = clientRect.bottom - clientRect.top;
	swapChainDescriptor.Format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDescriptor.SampleDesc.Count = 1;
	swapChainDescriptor.SampleDesc.Quality = 0;
	swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDescriptor.BufferCount = 2; // double buffered
	swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD; // if the contents of the back buffer should be preserved or discarded
	swapChainDescriptor.Scaling = DXGI_SCALING::DXGI_SCALING_STRETCH;
	swapChainDescriptor.Flags = {};

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDescriptor = {};
	swapChainFullscreenDescriptor.Windowed = true;

	if (FAILED(gfx.dxgiFactory->CreateSwapChainForHwnd(
		gfx.device.Get(),
		windowHandle,
		&swapChainDescriptor,
		&swapChainFullscreenDescriptor,
		nullptr,
		&gfx.swapChain)))
	{
		MessageBoxW(NULL, L"DXGI: Failed to create swapchain", L"Error", MB_ICONWARNING | MB_OK);
	}

	// Render target view

	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	if (FAILED(gfx.swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))))
	{
		MessageBoxW(nullptr, L"D3D11: Failed to get back buffer", L"Error", MB_ICONWARNING | MB_OK);
	}

	if (FAILED(gfx.device->CreateRenderTargetView(backBuffer.Get(), nullptr, &gfx.renderTarget)))
	{
		MessageBoxW(nullptr, L"D3D11: Failed to create render target view", L"Error", MB_ICONWARNING | MB_OK);
	}

	// viewport
	D3D11_VIEWPORT viewport {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(swapChainDescriptor.Width);
	viewport.Height = static_cast<float>(swapChainDescriptor.Height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	running = true;

	ShowWindow(windowHandle, SW_SHOW);
	
	while(running)
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
		gfx.deviceContext->ClearRenderTargetView(gfx.renderTarget.Get(), clearColour);
		gfx.deviceContext->RSSetViewports(1, &viewport);
		gfx.deviceContext->OMSetRenderTargets(1, gfx.renderTarget.GetAddressOf(), nullptr);

		gfx.swapChain->Present(1, 0);
	}

	return 0;
}