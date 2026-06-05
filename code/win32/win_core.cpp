#include "win_core.h"

#include "input/input.h"
#include "win_input.h"

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

        // mouse input
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        {
            bool mouseIsDown = (uMsg == WM_LBUTTONDOWN ||
                uMsg == WM_RBUTTONDOWN ||
                uMsg == WM_MBUTTONDOWN);

            Key key = (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP) ? Key::MOUSE_LEFT
                : (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONUP) ? Key::MOUSE_RIGHT
                : Key::MOUSE_MIDDLE;



            SysEvent event;
            event.type = mouseIsDown ? SysEventType::KEY_PRESS : SysEventType::KEY_RELEASE;
            event.key = key;
            event.position.x = (F32)(short)LOWORD(lParam);
            event.position.y = (F32)(short)HIWORD(lParam);

            Input::QueueSysEvent(event);
        }
        break;

        case WM_MOUSEMOVE:
        {
            SysEvent event;

            event.type = SysEventType::MOUSE_MOVE;
            event.key = Key::NONE;
            event.position.x = (F32)(short)LOWORD(lParam);
            event.position.y = (F32)(short)HIWORD(lParam);

            Input::QueueSysEvent(event);
        }
        break;

        // Keyboard input
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        {
            // NOTE:: function keys are syskeys when alt is held
            // alt by itself is allowed to fall through
            if (wParam != VK_MENU && wParam == VK_F4)
            {
                result = DefWindowProcW(hwnd, uMsg, wParam, lParam);
            }
        }
        [[fallthrough]];
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            bool keyWasDown { ((lParam) & (1 << 30)) != 0 };
            bool keyIsDown { ((lParam & (1 << 31)) == 0) };

            SysEvent event;

            event.type = keyIsDown ? SysEventType::KEY_PRESS : SysEventType::KEY_RELEASE;

            if (wParam < keyTable.size())
            {
                event.key = keyTable[wParam];
            }

            Input::QueueSysEvent(event);
        }
        break;

        case WM_MOUSEWHEEL:
        {
            SysEvent event;

            event.type = SysEventType::MOUSE_SCROLL;
            event.key = Key::NONE;
            event.scroll.x = 0;
            event.scroll.y = (F32)GET_WHEEL_DELTA_WPARAM(wParam) / (F32)WHEEL_DELTA;
            event.position.x = (F32)(short)LOWORD(lParam);
            event.position.y = (F32)(short)HIWORD(lParam);
            Input::QueueSysEvent(event);
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
    HWND WindowCreate(S32 width, S32 height, std::wstring_view title)
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

        // TODO:: This shouldn't live in this function but saves making a new function for now
        QueryPerformanceFrequency(&frequency);

        return handle;
    }

    RectF32 ClientRectFromWindow(const HWND handle)
    {
        RectF32 result;
        RECT clientRect {};

        if (GetClientRect(handle, &clientRect))
        {
            U32 width = clientRect.right - clientRect.left;
            U32 height = clientRect.bottom - clientRect.top;

            result.width = static_cast<F32>(width);
            result.height = static_cast<F32>(height);
            result.x = static_cast<F32>(clientRect.left);
            result.y = static_cast<F32>(clientRect.top);
        }

        return result;
    }

    U64 TimeMicroseconds()
    {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        F64 timeInSeconds { static_cast<F64>(currentTime.QuadPart) / static_cast<F64>(frequency.QuadPart) };
        return static_cast<U64>(timeInSeconds * 1000000.0);
    }

    F64 TimeSeconds()
    {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        F64 timeInSeconds { static_cast<F64>(currentTime.QuadPart) / static_cast<F64>(frequency.QuadPart) };
        return timeInSeconds;
    }
}