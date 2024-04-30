#include "../../Windows/Window.h"
#include "../../Windows/WindowsDefine.h"

#if PLATFORM_WINDOWS

#include "../../Windows/Keyboard.h"

namespace Infra
{
    bool Keyboard::IsKeyPressed(Key key)
    {
        int virtualKey = KeyCodeToWinVirtualKey(key);

        // GetAsyncKeyState() specifies whether the key was pressed
        // since the last call to GetAsyncKeyState(), and whether the
        // key is currently up or down. If the most significant bit
        // is set, the key is down, and if the least significant bit
        // is set, the key was pressed after the previous call
        // to GetAsyncKeyState().
        return (::GetAsyncKeyState(virtualKey) & 0x8000) != 0;
    }

    int Keyboard::KeyCodeToWinVirtualKey(Key key)
    {
        switch (key)
        {
            case Key::A:                return 'A';
            case Key::B:                return 'B';
            case Key::C:                return 'C';
            case Key::D:                return 'D';
            case Key::E:                return 'E';
            case Key::F:                return 'F';
            case Key::G:                return 'G';
            case Key::H:                return 'H';
            case Key::I:                return 'I';
            case Key::J:                return 'J';
            case Key::K:                return 'K';
            case Key::L:                return 'L';
            case Key::M:                return 'M';
            case Key::N:                return 'N';
            case Key::O:                return 'O';
            case Key::P:                return 'P';
            case Key::Q:                return 'Q';
            case Key::R:                return 'R';
            case Key::S:                return 'S';
            case Key::T:                return 'T';
            case Key::U:                return 'U';
            case Key::V:                return 'V';
            case Key::W:                return 'W';
            case Key::X:                return 'X';
            case Key::Y:                return 'Y';
            case Key::Z:                return 'Z';
            case Key::Num0:             return '0';
            case Key::Num1:             return '1';
            case Key::Num2:             return '2';
            case Key::Num3:             return '3';
            case Key::Num4:             return '4';
            case Key::Num5:             return '5';
            case Key::Num6:             return '6';
            case Key::Num7:             return '7';
            case Key::Num8:             return '8';
            case Key::Num9:             return '9';
            case Key::Escape:           return VK_ESCAPE;
            case Key::LControl:         return VK_LCONTROL;
            case Key::LShift:           return VK_LSHIFT;
            case Key::LAlt:             return VK_LMENU;
            case Key::LSystem:          return VK_LWIN;
            case Key::RControl:         return VK_RCONTROL;
            case Key::RShift:           return VK_RSHIFT;
            case Key::RAlt:             return VK_RMENU;
            case Key::RSystem:          return VK_RWIN;
            case Key::Menu:             return VK_APPS;
            case Key::LBracket:         return VK_OEM_4;
            case Key::RBracket:         return VK_OEM_6;
            case Key::Semicolon:        return VK_OEM_1;
            case Key::Comma:            return VK_OEM_COMMA;
            case Key::Period:           return VK_OEM_PERIOD;
            case Key::Apostrophe:       return VK_OEM_7;
            case Key::Slash:            return VK_OEM_2;
            case Key::Backslash:        return VK_OEM_5;
            case Key::Grave:            return VK_OEM_3;
            case Key::Equal:            return VK_OEM_PLUS;
            case Key::Minus:            return VK_OEM_MINUS;
            case Key::Space:            return VK_SPACE;
            case Key::Enter:            return VK_RETURN;
            case Key::Backspace:        return VK_BACK;
            case Key::Tab:              return VK_TAB;
            case Key::PageUp:           return VK_PRIOR;
            case Key::PageDown:         return VK_NEXT;
            case Key::End:              return VK_END;
            case Key::Home:             return VK_HOME;
            case Key::Insert:           return VK_INSERT;
            case Key::Delete:           return VK_DELETE;
            case Key::NumPadAdd:        return VK_ADD;
            case Key::NumPadMinus:      return VK_SUBTRACT;
            case Key::NumPadMultiply:   return VK_MULTIPLY;
            case Key::NumPadDivide:     return VK_DIVIDE;
            case Key::ArrowLeft:        return VK_LEFT;
            case Key::ArrowRight:       return VK_RIGHT;
            case Key::ArrowUp:          return VK_UP;
            case Key::ArrowDown:        return VK_DOWN;
            case Key::NumPad0:          return VK_NUMPAD0;
            case Key::NumPad1:          return VK_NUMPAD1;
            case Key::NumPad2:          return VK_NUMPAD2;
            case Key::NumPad3:          return VK_NUMPAD3;
            case Key::NumPad4:          return VK_NUMPAD4;
            case Key::NumPad5:          return VK_NUMPAD5;
            case Key::NumPad6:          return VK_NUMPAD6;
            case Key::NumPad7:          return VK_NUMPAD7;
            case Key::NumPad8:          return VK_NUMPAD8;
            case Key::NumPad9:          return VK_NUMPAD9;
            case Key::F1:               return VK_F1;
            case Key::F2:               return VK_F2;
            case Key::F3:               return VK_F3;
            case Key::F4:               return VK_F4;
            case Key::F5:               return VK_F5;
            case Key::F6:               return VK_F6;
            case Key::F7:               return VK_F7;
            case Key::F8:               return VK_F8;
            case Key::F9:               return VK_F9;
            case Key::F10:              return VK_F10;
            case Key::F11:              return VK_F11;
            case Key::F12:              return VK_F12;
            case Key::F13:              return VK_F13;
            case Key::F14:              return VK_F14;
            case Key::F15:              return VK_F15;
            case Key::Pause:            return VK_PAUSE;
            default:                    return 0;
        }
    }

    Keyboard::Key Keyboard::WinVirtualKeyToKeyCode(int virtualKey, void* lParam)
    {
        LPARAM flags = reinterpret_cast<LPARAM>(lParam);
        switch (virtualKey)
        {
            case VK_SHIFT:
            {
                static UINT lShift = ::MapVirtualKeyW(VK_LSHIFT, MAPVK_VK_TO_VSC);
                UINT scancode = static_cast<UINT>((flags & (0xFF << 16)) >> 16);
                return scancode == lShift ? Keyboard::Key::LShift : Keyboard::Key::RShift;
            }
            case VK_MENU :
                return (HIWORD(flags) & KF_EXTENDED) ? Keyboard::Key::RAlt : Keyboard::Key::LAlt;
            case VK_CONTROL :
                return (HIWORD(flags) & KF_EXTENDED) ? Keyboard::Key::RControl : Keyboard::Key::LControl;
            case VK_LWIN:       return Keyboard::Key::LSystem;
            case VK_RWIN:       return Keyboard::Key::RSystem;
            case VK_APPS:       return Keyboard::Key::Menu;
            case VK_OEM_1:      return Keyboard::Key::Semicolon;
            case VK_OEM_2:      return Keyboard::Key::Slash;
            case VK_OEM_PLUS:   return Keyboard::Key::Equal;
            case VK_OEM_MINUS:  return Keyboard::Key::Minus;
            case VK_OEM_4:      return Keyboard::Key::LBracket;
            case VK_OEM_6:      return Keyboard::Key::RBracket;
            case VK_OEM_COMMA:  return Keyboard::Key::Comma;
            case VK_OEM_PERIOD: return Keyboard::Key::Period;
            case VK_OEM_7:      return Keyboard::Key::Apostrophe;
            case VK_OEM_5:      return Keyboard::Key::Backslash;
            case VK_OEM_3:      return Keyboard::Key::Grave;
            case VK_ESCAPE:     return Keyboard::Key::Escape;
            case VK_SPACE:      return Keyboard::Key::Space;
            case VK_RETURN:     return Keyboard::Key::Enter;
            case VK_BACK:       return Keyboard::Key::Backspace;
            case VK_TAB:        return Keyboard::Key::Tab;
            case VK_PRIOR:      return Keyboard::Key::PageUp;
            case VK_NEXT:       return Keyboard::Key::PageDown;
            case VK_END:        return Keyboard::Key::End;
            case VK_HOME:       return Keyboard::Key::Home;
            case VK_INSERT:     return Keyboard::Key::Insert;
            case VK_DELETE:     return Keyboard::Key::Delete;
            case VK_ADD:        return Keyboard::Key::NumPadAdd;
            case VK_SUBTRACT:   return Keyboard::Key::NumPadMinus;
            case VK_MULTIPLY:   return Keyboard::Key::NumPadMultiply;
            case VK_DIVIDE:     return Keyboard::Key::NumPadDivide;
            case VK_PAUSE:      return Keyboard::Key::Pause;
            case VK_F1:         return Keyboard::Key::F1;
            case VK_F2:         return Keyboard::Key::F2;
            case VK_F3:         return Keyboard::Key::F3;
            case VK_F4:         return Keyboard::Key::F4;
            case VK_F5:         return Keyboard::Key::F5;
            case VK_F6:         return Keyboard::Key::F6;
            case VK_F7:         return Keyboard::Key::F7;
            case VK_F8:         return Keyboard::Key::F8;
            case VK_F9:         return Keyboard::Key::F9;
            case VK_F10:        return Keyboard::Key::F10;
            case VK_F11:        return Keyboard::Key::F11;
            case VK_F12:        return Keyboard::Key::F12;
            case VK_F13:        return Keyboard::Key::F13;
            case VK_F14:        return Keyboard::Key::F14;
            case VK_F15:        return Keyboard::Key::F15;
            case VK_LEFT:       return Keyboard::Key::ArrowLeft;
            case VK_RIGHT:      return Keyboard::Key::ArrowRight;
            case VK_UP:         return Keyboard::Key::ArrowUp;
            case VK_DOWN:       return Keyboard::Key::ArrowDown;
            case VK_NUMPAD0:    return Keyboard::Key::NumPad0;
            case VK_NUMPAD1:    return Keyboard::Key::NumPad1;
            case VK_NUMPAD2:    return Keyboard::Key::NumPad2;
            case VK_NUMPAD3:    return Keyboard::Key::NumPad3;
            case VK_NUMPAD4:    return Keyboard::Key::NumPad4;
            case VK_NUMPAD5:    return Keyboard::Key::NumPad5;
            case VK_NUMPAD6:    return Keyboard::Key::NumPad6;
            case VK_NUMPAD7:    return Keyboard::Key::NumPad7;
            case VK_NUMPAD8:    return Keyboard::Key::NumPad8;
            case VK_NUMPAD9:    return Keyboard::Key::NumPad9;
            case 'A':           return Keyboard::Key::A;
            case 'Z':           return Keyboard::Key::Z;
            case 'E':           return Keyboard::Key::E;
            case 'R':           return Keyboard::Key::R;
            case 'T':           return Keyboard::Key::T;
            case 'Y':           return Keyboard::Key::Y;
            case 'U':           return Keyboard::Key::U;
            case 'I':           return Keyboard::Key::I;
            case 'O':           return Keyboard::Key::O;
            case 'P':           return Keyboard::Key::P;
            case 'Q':           return Keyboard::Key::Q;
            case 'S':           return Keyboard::Key::S;
            case 'D':           return Keyboard::Key::D;
            case 'F':           return Keyboard::Key::F;
            case 'G':           return Keyboard::Key::G;
            case 'H':           return Keyboard::Key::H;
            case 'J':           return Keyboard::Key::J;
            case 'K':           return Keyboard::Key::K;
            case 'L':           return Keyboard::Key::L;
            case 'M':           return Keyboard::Key::M;
            case 'W':           return Keyboard::Key::W;
            case 'X':           return Keyboard::Key::X;
            case 'C':           return Keyboard::Key::C;
            case 'V':           return Keyboard::Key::V;
            case 'B':           return Keyboard::Key::B;
            case 'N':           return Keyboard::Key::N;
            case '0':           return Keyboard::Key::Num0;
            case '1':           return Keyboard::Key::Num1;
            case '2':           return Keyboard::Key::Num2;
            case '3':           return Keyboard::Key::Num3;
            case '4':           return Keyboard::Key::Num4;
            case '5':           return Keyboard::Key::Num5;
            case '6':           return Keyboard::Key::Num6;
            case '7':           return Keyboard::Key::Num7;
            case '8':           return Keyboard::Key::Num8;
            case '9':           return Keyboard::Key::Num9;
            default:            return Keyboard::Key::Unknown;
        }
    }


}

#endif