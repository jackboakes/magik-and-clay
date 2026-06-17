#include "input.h"

namespace Input
{
    void QueueSysEvent(SysEvent sysEvent)
    {
        sysEventBuffer.Write(sysEvent);
    }

    // TODO:: Unsure if this is the final resting place for this code
    void ProcessEvents()
    {
        keyStatePrevious = keyStateCurrent;
        while(!sysEventBuffer.Empty())
        {
            SysEvent event { sysEventBuffer.Read() };
            switch (event.type)
            {
                //TODO:: This is temporary as the user can skip keys if the user presses and releases on the same frame.
                case SysEventType::KEY_PRESS:
                {
                    keyStateCurrent[static_cast<size_t>(event.key)] = true;
                }
                break;
                case SysEventType::KEY_RELEASE:
                {
                    keyStateCurrent[static_cast<size_t>(event.key)] = false;
                }
                break;
                case SysEventType::MOUSE_SCROLL:
                {
                    Input::scrollDelta += event.scroll.y;
                }
                break;
                case SysEventType::MOUSE_MOVE:
                {
                    Input::mousePosition = event.position;
                }
                break;
            }
        }
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

    F32 GetScrollDelta()
    {
        F32 delta = scrollDelta;
        scrollDelta = 0.0f;
        return delta;
    }

    Vec2F32 GetMousePosition()
    {
        return mousePosition;
    }
}
