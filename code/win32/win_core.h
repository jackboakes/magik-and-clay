#pragma once

#include "common/types.h"

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

namespace W32
{
    // TODO:: handle this somewhere else
    inline bool running;

    static LARGE_INTEGER frequency;

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    // TODO:: Pass in a rect type?
    HWND WindowCreate(S32 width, S32 height, std::wstring_view title);

    RectF32 ClientRectFromWindow(const HWND handle);

    U64 TimeMicroseconds();
    F64 TimeSeconds();
}