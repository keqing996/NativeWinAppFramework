#pragma once

#include <utility>

namespace NWA
{
    class Mouse
    {
    public:
        Mouse() = delete;

    public:
        enum class Button
        {
            Left,
            Right,
            Middle,
            Addition1,
            Addition2
        };

    public:
        static bool IsButtonPressed(Button button);
        static std::pair<int, int> GetPosition();
        static void SetPosition(const std::pair<int, int>& newPos);
        static void SetPosition(int newPosX, int newPosY);
    };

}
