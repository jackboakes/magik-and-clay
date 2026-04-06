#pragma once

#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <string_view>

namespace W32
{
    inline bool running;

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    // TODO:: Pass in a rect type?
    HWND WindowCreate(int width, int height, std::wstring_view title);

    // TODO:: add microsecond timer with QueryPerformanceFrequency and QueryPerformanceCounter
}