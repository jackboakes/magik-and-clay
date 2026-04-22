#include "input.h"

namespace Input
{
    void QueueSysEvent(SysEvent sysEvent)
    {
        sysEventQueue.push_back(sysEvent);
    }

    constexpr std::string StringFromKey(Key key)
    {
        return keyNames[static_cast<size_t>(key)];
    }

    bool IsKeyDown(Key key)
    {
        return keyStateCurrent[static_cast<size_t>(key)];
    }

    bool IsKeyPressed(Key key)
    {
        return keyStateCurrent[static_cast<size_t>(key)] &&
            !keyStatePrevious[static_cast<size_t>(key)];
    }

    bool IsKeyReleased(Key key)
    {
        return !keyStateCurrent[static_cast<size_t>(key)] &&
            keyStatePrevious[static_cast<size_t>(key)];
    }

    float GetScrollDelta()
    {
        float delta = scrollDelta;
        scrollDelta = 0.0f;
        return delta;
    }

    HMM_Vec2 GetMousePosition()
    {
        return mousePosition;
    }
}
