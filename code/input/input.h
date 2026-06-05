#pragma once

#include "common/types.h"

#include <array>
#include <string>
#include <vector>


enum class SysEventType
{
    NONE = 0,
    KEY_PRESS,
    KEY_RELEASE,
    MOUSE_MOVE,
    MOUSE_SCROLL
};

struct SysEvent
{
    SysEventType type;
    Key key;

    Vec2F32 position;
    Vec2F32 scroll;
};

// TODO:: replace this with a fixed size ring buffer can't have heap alloc in hot path
inline std::vector<SysEvent> sysEventQueue;

// push keys from events into here
inline std::array<bool, static_cast<size_t>(Key::COUNT)> keyStateCurrent {};
inline std::array<bool, static_cast<size_t>(Key::COUNT)> keyStatePrevious {};



namespace Input
{
    inline F32 scrollDelta;
    inline Vec2F32 mousePosition;

    constexpr std::array<const char*, static_cast<size_t>(Key::COUNT)> keyNames = []()
        {
            std::array<const char*, static_cast<size_t>(Key::COUNT)> r {};

            r[static_cast<size_t>(Key::NONE)] = "NONE";
            r[static_cast<size_t>(Key::A)] = "A";
            r[static_cast<size_t>(Key::B)] = "B";
            r[static_cast<size_t>(Key::C)] = "C";
            r[static_cast<size_t>(Key::D)] = "D";
            r[static_cast<size_t>(Key::E)] = "E";
            r[static_cast<size_t>(Key::F)] = "F";
            r[static_cast<size_t>(Key::G)] = "G";
            r[static_cast<size_t>(Key::H)] = "H";
            r[static_cast<size_t>(Key::I)] = "I";
            r[static_cast<size_t>(Key::J)] = "J";
            r[static_cast<size_t>(Key::K)] = "K";
            r[static_cast<size_t>(Key::L)] = "L";
            r[static_cast<size_t>(Key::M)] = "M";
            r[static_cast<size_t>(Key::N)] = "N";
            r[static_cast<size_t>(Key::O)] = "O";
            r[static_cast<size_t>(Key::P)] = "P";
            r[static_cast<size_t>(Key::Q)] = "Q";
            r[static_cast<size_t>(Key::R)] = "R";
            r[static_cast<size_t>(Key::S)] = "S";
            r[static_cast<size_t>(Key::T)] = "T";
            r[static_cast<size_t>(Key::U)] = "U";
            r[static_cast<size_t>(Key::V)] = "V";
            r[static_cast<size_t>(Key::W)] = "W";
            r[static_cast<size_t>(Key::X)] = "X";
            r[static_cast<size_t>(Key::Y)] = "Y";
            r[static_cast<size_t>(Key::Z)] = "Z";
            r[static_cast<size_t>(Key::NUMPAD0)] = "NUMPAD0";
            r[static_cast<size_t>(Key::NUMPAD1)] = "NUMPAD1";
            r[static_cast<size_t>(Key::NUMPAD2)] = "NUMPAD2";
            r[static_cast<size_t>(Key::NUMPAD3)] = "NUMPAD3";
            r[static_cast<size_t>(Key::NUMPAD4)] = "NUMPAD4";
            r[static_cast<size_t>(Key::NUMPAD5)] = "NUMPAD5";
            r[static_cast<size_t>(Key::NUMPAD6)] = "NUMPAD6";
            r[static_cast<size_t>(Key::NUMPAD7)] = "NUMPAD7";
            r[static_cast<size_t>(Key::NUMPAD8)] = "NUMPAD8";
            r[static_cast<size_t>(Key::NUMPAD9)] = "NUMPAD9";
            r[static_cast<size_t>(Key::F1)] = "F1";
            r[static_cast<size_t>(Key::F2)] = "F2";
            r[static_cast<size_t>(Key::F3)] = "F3";
            r[static_cast<size_t>(Key::F4)] = "F4";
            r[static_cast<size_t>(Key::F5)] = "F5";
            r[static_cast<size_t>(Key::F6)] = "F6";
            r[static_cast<size_t>(Key::F7)] = "F7";
            r[static_cast<size_t>(Key::F8)] = "F8";
            r[static_cast<size_t>(Key::F9)] = "F9";
            r[static_cast<size_t>(Key::F10)] = "F10";
            r[static_cast<size_t>(Key::F11)] = "F11";
            r[static_cast<size_t>(Key::F12)] = "F12";
            r[static_cast<size_t>(Key::GRAVE)] = "GRAVE";
            r[static_cast<size_t>(Key::ZERO)] = "0";
            r[static_cast<size_t>(Key::ONE)] = "1";
            r[static_cast<size_t>(Key::TWO)] = "2";
            r[static_cast<size_t>(Key::THREE)] = "3";
            r[static_cast<size_t>(Key::FOUR)] = "4";
            r[static_cast<size_t>(Key::FIVE)] = "5";
            r[static_cast<size_t>(Key::SIX)] = "6";
            r[static_cast<size_t>(Key::SEVEN)] = "7";
            r[static_cast<size_t>(Key::EIGHT)] = "8";
            r[static_cast<size_t>(Key::NINE)] = "9";
            r[static_cast<size_t>(Key::ESCAPE)] = "ESCAPE";
            r[static_cast<size_t>(Key::SPACE)] = "SPACE";
            r[static_cast<size_t>(Key::ENTER)] = "ENTER";
            r[static_cast<size_t>(Key::TAB)] = "TAB";
            r[static_cast<size_t>(Key::BACKSPACE)] = "BACKSPACE";
            r[static_cast<size_t>(Key::DEL)] = "DELETE";
            r[static_cast<size_t>(Key::INSERT)] = "INSERT";
            r[static_cast<size_t>(Key::LEFT)] = "LEFT";
            r[static_cast<size_t>(Key::RIGHT)] = "RIGHT";
            r[static_cast<size_t>(Key::UP)] = "UP";
            r[static_cast<size_t>(Key::DOWN)] = "DOWN";
            r[static_cast<size_t>(Key::HOME)] = "HOME";
            r[static_cast<size_t>(Key::END)] = "END";
            r[static_cast<size_t>(Key::PAGE_UP)] = "PAGE_UP";
            r[static_cast<size_t>(Key::PAGE_DOWN)] = "PAGE_DOWN";
            r[static_cast<size_t>(Key::CAPS_LOCK)] = "CAPS_LOCK";
            r[static_cast<size_t>(Key::SHIFT)] = "SHIFT";
            r[static_cast<size_t>(Key::CTRL)] = "CTRL";
            r[static_cast<size_t>(Key::ALT)] = "ALT";
            r[static_cast<size_t>(Key::MINUS)] = "MINUS";
            r[static_cast<size_t>(Key::EQUALS)] = "EQUALS";
            r[static_cast<size_t>(Key::LBRACKET)] = "LBRACKET";
            r[static_cast<size_t>(Key::RBRACKET)] = "RBRACKET";
            r[static_cast<size_t>(Key::BACKSLASH)] = "BACKSLASH";
            r[static_cast<size_t>(Key::SEMICOLON)] = "SEMICOLON";
            r[static_cast<size_t>(Key::APOSTROPHE)] = "APOSTROPHE";
            r[static_cast<size_t>(Key::COMMA)] = "COMMA";
            r[static_cast<size_t>(Key::PERIOD)] = "PERIOD";
            r[static_cast<size_t>(Key::SLASH)] = "SLASH";
            r[static_cast<size_t>(Key::NUMPAD_ADD)] = "NUMPAD_ADD";
            r[static_cast<size_t>(Key::NUMPAD_SUB)] = "NUMPAD_SUB";
            r[static_cast<size_t>(Key::NUMPAD_MUL)] = "NUMPAD_MUL";
            r[static_cast<size_t>(Key::NUMPAD_DIV)] = "NUMPAD_DIV";
            r[static_cast<size_t>(Key::NUMPAD_DECIMAL)] = "NUMPAD_DECIMAL";
            r[static_cast<size_t>(Key::NUM_LOCK)] = "NUM_LOCK";
            r[static_cast<size_t>(Key::PRINT_SCREEN)] = "PRINT_SCREEN";
            r[static_cast<size_t>(Key::PAUSE)] = "PAUSE";
            r[static_cast<size_t>(Key::MENU)] = "MENU";
            r[static_cast<size_t>(Key::MOUSE_LEFT)] = "MOUSE_LEFT";
            r[static_cast<size_t>(Key::MOUSE_RIGHT)] = "MOUSE_RIGHT";
            r[static_cast<size_t>(Key::MOUSE_MIDDLE)] = "MOUSE_MIDDLE";

            return r;
        }();

        void QueueSysEvent(SysEvent sysEvent);
        void ProcessEvents();

        constexpr std::string StringFromKey(Key key);

        // - High level API that the game calls
        bool IsKeyDown(Key key);
        bool IsKeyPressed(Key key);
        bool IsKeyReleased(Key key);

        F32 GetScrollDelta();
        Vec2F32 GetMousePosition();
}


