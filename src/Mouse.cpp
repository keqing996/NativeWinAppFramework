#include "../../src/Infra/Windows/WindowsDefine.hpp"
#include "../../src/NativeWinApp/Window.h"
#include "../../src/NativeWinApp/Mouse.h"

namespace NWA
{
    bool Mouse::IsButtonPressed(Button button)
    {
        int virtualKey = 0;
        switch (button)
        {
            case Button::Left:
                virtualKey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON;
                break;
            case Button::Right:
                virtualKey = GetSystemMetrics(SM_SWAPBUTTON) ? VK_LBUTTON : VK_RBUTTON;
                break;
            case Button::Middle:
                virtualKey = VK_MBUTTON;
                break;
            case Button::Addition1:
                virtualKey = VK_XBUTTON1;
                break;
            case Button::Addition2:
                virtualKey = VK_XBUTTON2;
                break;
            default:
                virtualKey = 0;
                break;
        }

        // GetAsyncKeyState() specifies whether the key was pressed
        // since the last call to GetAsyncKeyState(), and whether the
        // key is currently up or down. If the most significant bit
        // is set, the key is down, and if the least significant bit
        // is set, the key was pressed after the previous call
        // to GetAsyncKeyState().
        return (::GetAsyncKeyState(virtualKey) & 0x8000) != 0;
    }

    std::pair<int, int> Mouse::GetPosition()
    {
        POINT point;
        ::GetCursorPos(&point);
        return { point.x, point.y };
    }

    void Mouse::SetPosition(const std::pair<int, int>& newPos)
    {
        ::SetCursorPos(newPos.first, newPos.second);
    }

    void Mouse::SetPosition(int newPosX, int newPosY)
    {
        ::SetCursorPos(newPosX, newPosY);
    }

}