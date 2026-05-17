#pragma once

#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#undef DrawText 
#undef DrawTextA
#undef DrawTextW
#undef DrawTextEx 
#undef DrawTextExA
#undef DrawTextExW
#undef min
#undef max

#include <string_view>
#include "common/types.h"

namespace W32
{
    // TODO:: handle this somewhere else
    inline bool running;

    static LARGE_INTEGER frequency;

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    // TODO:: Pass in a rect type?
    HWND WindowCreate(int width, int height, std::wstring_view title);

    RectF32 ClientRectFromWindow(const HWND handle);

    uint64_t TimeMicroseconds();
    double TimeSeconds();
}