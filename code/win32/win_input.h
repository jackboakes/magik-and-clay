#pragma once

#include "common/types.h"

#include <array>
#include <windows.h>

constexpr auto KeyFromVK()
{
    std::array<Key, 256> result {};

    for (int i { 'A' }; i <= 'Z'; i++)
    {
        result[i] = static_cast<Key>(static_cast<int>(Key::A) + (i - 'A'));
    }

    for (int i { '0' }; i <= '9'; i++)
    {
        result[i] = static_cast<Key>(static_cast<int>(Key::ZERO) + (i - '0'));
    }

    for (int i { VK_NUMPAD0 }; i <= VK_NUMPAD9; i++)
    {
        result[i] = static_cast<Key>(static_cast<int>(Key::NUMPAD0) + (i - VK_NUMPAD0));
    }

    for (int i { VK_F1 }; i <= VK_F12; i++)
    {
        result[i] = static_cast<Key>(static_cast<int>(Key::F1) + (i - VK_F1));
    }


    result[VK_SPACE] = Key::SPACE;
    result[VK_RETURN] = Key::ENTER;
    result[VK_TAB] = Key::TAB;
    result[VK_BACK] = Key::BACKSPACE;
    result[VK_DELETE] = Key::DEL;
    result[VK_INSERT] = Key::INSERT;

    result[VK_LEFT] = Key::LEFT;
    result[VK_RIGHT] = Key::RIGHT;
    result[VK_UP] = Key::UP;
    result[VK_DOWN] = Key::DOWN;

    result[VK_HOME] = Key::HOME;
    result[VK_END] = Key::END;
    result[VK_PRIOR] = Key::PAGE_UP;
    result[VK_NEXT] = Key::PAGE_DOWN;

    result[VK_CAPITAL] = Key::CAPS_LOCK;
    result[VK_SHIFT] = Key::SHIFT;
    result[VK_LSHIFT] = Key::SHIFT;
    result[VK_RSHIFT] = Key::SHIFT;
    result[VK_CONTROL] = Key::CTRL;
    result[VK_LCONTROL] = Key::CTRL;
    result[VK_RCONTROL] = Key::CTRL;
    result[VK_MENU] = Key::ALT;
    result[VK_LMENU] = Key::ALT;
    result[VK_RMENU] = Key::ALT;

    result[VK_OEM_MINUS] = Key::MINUS;
    result[VK_OEM_PLUS] = Key::EQUALS;
    result[VK_OEM_4] = Key::LBRACKET;
    result[VK_OEM_6] = Key::RBRACKET;
    result[VK_OEM_5] = Key::BACKSLASH;
    result[VK_OEM_1] = Key::SEMICOLON;
    result[VK_OEM_7] = Key::APOSTROPHE;
    result[VK_OEM_COMMA] = Key::COMMA;
    result[VK_OEM_PERIOD] = Key::PERIOD;
    result[VK_OEM_2] = Key::SLASH;
    result[VK_OEM_3] = Key::GRAVE;

    result[VK_ADD] = Key::NUMPAD_ADD;
    result[VK_SUBTRACT] = Key::NUMPAD_SUB;
    result[VK_MULTIPLY] = Key::NUMPAD_MUL;
    result[VK_DIVIDE] = Key::NUMPAD_DIV;
    result[VK_DECIMAL] = Key::NUMPAD_DECIMAL;

    result[VK_NUMLOCK] = Key::NUM_LOCK;
    result[VK_SNAPSHOT] = Key::PRINT_SCREEN;
    result[VK_ESCAPE] = Key::ESCAPE;
    result[VK_PAUSE] = Key::PAUSE;
    result[VK_APPS] = Key::MENU;

    result[VK_LBUTTON] = Key::MOUSE_LEFT;
    result[VK_RBUTTON] = Key::MOUSE_RIGHT;
    result[VK_MBUTTON] = Key::MOUSE_MIDDLE;

    return result;
}

constexpr auto keyTable { KeyFromVK() };