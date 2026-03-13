#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>

LRESULT CALLBACK Win32MainWindowCallback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


//  if you forget wtf any of this is just read the documentation back bro
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	WNDCLASSW wc {};
	wc.lpfnWndProc = Win32MainWindowCallback;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"GameWindow";
	wc.style = CS_OWNDC;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	if (!RegisterClassW(&wc))
	{
		return 0;
	}

	HWND hwnd = CreateWindowExW(
		0,											// Optional window styles
		L"GameWindow",								// Window class
		L"Clay & Magik",							// Window title
		WS_OVERLAPPEDWINDOW,						// Window style
		CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,	// Size and position
		NULL,										// Parent window
		NULL,										// Menu
		hInstance,									// Instance handle
		NULL										// Additional application data
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);

	MSG msg {};

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return static_cast<int>(msg.wParam);
}