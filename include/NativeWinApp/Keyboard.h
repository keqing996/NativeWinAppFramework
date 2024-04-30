#pragma once

namespace Infra
{

    class Keyboard
    {
    public:
        Keyboard() = delete;

    public:
        enum class Key: int
        {
            Unknown = -1,
            Q, W, E, R, T, Y, U, I, O, P,
            A, S, D, F, G, H, J, K, L,
            Z, X, C, V, B, N, M,
            F1, F2, F3, F4, F5, F6, F7, F8, F9,
            F10, F11, F12, F13, F14, F15,
            ArrowUp, ArrowDown, ArrowLeft, ArrowRight,
            Num0, Num1, Num2, Num3, Num4,
            Num5, Num6, Num7, Num8, Num9,
            NumPad0, NumPad1, NumPad2, NumPad3, NumPad4,
            NumPad5, NumPad6, NumPad7, NumPad8, NumPad9,
            LControl, LAlt, LShift, LSystem,
            RControl, RAlt, RShift, RSystem,
            Escape, Menu, LBracket, RBracket,
            Space, Enter, Backspace, Tab,
            PageUp, PageDown, Home, End, Delete,
            Insert, Minus, Equal, Pause,
            Semicolon /* ; */, Comma /* , */,
            Period /* . */, Apostrophe /* ' */,
            Slash /* / */, Backslash /* \ */, Grave /* ` */,
            NumPadMinus, NumPadAdd, NumPadMultiply, NumPadDivide,
            Count
        };

    public:
        static bool IsKeyPressed(Key key);
        static int KeyCodeToWinVirtualKey(Key key);
        static Key WinVirtualKeyToKeyCode(int virtualKey, void* lParam);
    };


}
