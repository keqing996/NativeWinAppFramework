#pragma once

#include "Keyboard.h"
#include "Mouse.h"

namespace NWA
{
    struct WindowEvent
    {
    public:
        enum class Type
        {
            None,
            // No struct event
            Close,
            GetFocus,
            LostFocus,
            MouseEnter,
            MouseLeave,
            // Have struct event
            Resize,
            Text,
            KeyPressed,
            KeyReleased,
            MouseWheel,
            MouseButtonPressed,
            MouseButtonReleased,
            MouseMoved,
        };

        struct SizeData
        {
            unsigned int width;
            unsigned int height;
        };

        struct KeyData
        {
            Keyboard::Key key;
            bool alt;
            bool control;
            bool shift;
            bool system;
        };

        struct MouseMoveData
        {
            int x;
            int y;
        };

        struct MouseButtonData
        {
            Mouse::Button button;
            int x;
            int y;
        };

        struct MouseWheelData
        {
            int delta;
            int x;
            int y;
        };

        struct TextData
        {
            uint32_t utf;
        };

        union Data
        {
            SizeData sizeData;
            KeyData keyData;
            MouseMoveData mouseMoveData;
            MouseButtonData mouseButtonData;
            MouseWheelData mouseWheelData;
            TextData textData;
        };

    public:
        Type type;
        Data data;

    public:
        explicit WindowEvent(Type t)
            : type(t)
        {
        }
    };
}
