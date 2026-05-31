#pragma once
#include <cstdint>

struct Vec2S32
{
    int32_t x;
    int32_t y;

    Vec2S32 operator+(const Vec2S32& other) const
    {
        return { x + other.x, y + other.y };
    }

    Vec2S32 operator-(const Vec2S32& other) const
    {
        return { x - other.x, y - other.y };
    }

    bool operator==(const Vec2S32& other) const
    {
        return x == other.x && y == other.y;
    }
};

struct Vec2F32
{
    float x;
    float y;

    Vec2F32 operator+(const Vec2F32& other) const
    {
        return { x + other.x, y + other.y };
    }

    Vec2F32 operator-(const Vec2F32& other) const
    {
        return { x - other.x, y - other.y };
    }
};

struct RectF32
{
    float x;
    float y;
    float width;
    float height;
};

enum class Key
{
    NONE = 0,
    ZERO,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    NUMPAD0,
    NUMPAD1,
    NUMPAD2,
    NUMPAD3,
    NUMPAD4,
    NUMPAD5,
    NUMPAD6,
    NUMPAD7,
    NUMPAD8,
    NUMPAD9,
    GRAVE,
    ESCAPE,
    SPACE, ENTER, TAB, BACKSPACE, DEL, INSERT,
    LEFT, RIGHT, UP, DOWN,
    HOME, END, PAGE_UP, PAGE_DOWN,
    CAPS_LOCK, SHIFT, CTRL, ALT,
    MINUS, EQUALS,
    LBRACKET, RBRACKET,
    BACKSLASH, SEMICOLON, APOSTROPHE, COMMA, PERIOD, SLASH,
    NUMPAD_ADD, NUMPAD_SUB, NUMPAD_MUL, NUMPAD_DIV,
    NUMPAD_DECIMAL, NUM_LOCK,
    PRINT_SCREEN, PAUSE, MENU,
    MOUSE_LEFT, MOUSE_RIGHT, MOUSE_MIDDLE,
    COUNT
};