#include "win_core.h"

namespace W32 
{
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

    // TODO:: Pass in a rect type?
    HWND WindowCreate(int width, int height, std::wstring_view title)
    {
        HINSTANCE hInstance = GetModuleHandleW(0);

        // register window class
        {
            WNDCLASSW windowClass {};
            windowClass.lpfnWndProc = WindowProc;
            windowClass.hInstance = hInstance;
            windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
            windowClass.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
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

    RectF32 ClientRectFromWindow(const HWND handle)
    {
        RectF32 result;
        RECT clientRect {};

        if (GetClientRect(handle, &clientRect))
        {
            UINT width = clientRect.right - clientRect.left;
            UINT height = clientRect.bottom - clientRect.top;

            result.width = static_cast<float>(width);
            result.height = static_cast<float>(height);
            result.x = static_cast<float>(clientRect.left);
            result.y = static_cast<float>(clientRect.top);
        }

        return result;
    }
}