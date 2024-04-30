#include "../../Windows/Window.h"
#include "../../Windows/WindowsDefine.h"

#if PLATFORM_WINDOWS

#include "../../ScopeGuard.hpp"
#include "../../String.hpp"
#include "../../Logger.hpp"
#include "../../Windows/WindowEvent.h"

namespace Infra
{
    void Window::WindowEventProcess(uint32_t message, void* wpara, void* lpara)
    {
        WindowEventProcessInternal(message, wpara, lpara);

        if (_winEventProcess)
            _winEventProcess(message, wpara, lpara);
    }

    void Window::WindowEventProcessInternal(uint32_t message, void* wpara, void* lpara)
    {
        if (_hWindow == nullptr)
            return;

        WPARAM wParam = reinterpret_cast<WPARAM>(wpara);
        LPARAM lParam = reinterpret_cast<LPARAM>(lpara);

        switch (message)
        {
            case WM_DESTROY:
            {
                // Do nothing for now.
                break;
            }
            case WM_CLOSE:
            {
                WindowEvent event(WindowEvent::Type::Close);
                PushEvent(event);
                break;
            }
            case WM_SETCURSOR:
            {
                // lower world of lParam is hit test result
                if (LOWORD(lParam) == HTCLIENT)
                    ::SetCursor(_cursorVisible ? reinterpret_cast<HCURSOR>(_hCursor) : nullptr);

                break;
            }
            case WM_SIZE:
            {
                auto newSize = GetSize();
                if (wParam != SIZE_MINIMIZED && _windowSize != newSize)
                {
                    _windowSize = newSize;

                    WindowEvent event(WindowEvent::Type::Resize);
                    event.data.sizeData.width = _windowSize.first;
                    event.data.sizeData.height = _windowSize.second;
                    PushEvent(event);
                }
                break;
            }
            case WM_SETFOCUS:
            {
                CaptureCursorInternal(_cursorCapture);

                WindowEvent event(WindowEvent::Type::GetFocus);
                PushEvent(event);
                break;
            }
            case WM_KILLFOCUS:
            {
                CaptureCursorInternal(false);

                WindowEvent event(WindowEvent::Type::LostFocus);
                PushEvent(event);
                break;
            }
            case WM_CHAR:
            {
                // IME: input method editor
                // No IME: input '9' -> WM_CHAR(0x0039)
                // With IME: input '9' -> WM_IME_CHAR(0x0039) -> WM_CHAR(0x0039)
                // With IME: input '笨' -> WM_IME_CHAR(0xB1BF) -> WM_CHAR(0x00B1) -> WM_CHAR(0x00BF)
                // In BMP, the Code Point segments from U+D800 to U+DFFF are permanently retained and not mapped to characters.
                // Apart from BMP, the first two bytes of the four bytes are high bytes, and the last two bytes are low bytes.
                // The range of the first two bytes is 0xD800 to 0xDBFF.
                // The range of the last two bytes is 0xDC00 to 0xDFFF.
                // It can be seen that there is no intersection between characters in BMP and those other than BMP.
                // Therefore, if (character >= 0xD800 && character <= 0xDBFF)
                // If this condition is met, it indicates the uncommon words that occupy 4 bytes.
                // For uncommon words, the system will send two wm_ime_char messages, the first to send its high byte, the second to send its low byte
                static uint32_t imeFirstWideChar;
                if (_enableKeyRepeat || ((HIWORD(lParam) & KF_REPEAT) == 0))
                {
                    auto character = static_cast<uint32_t>(wParam);
                    // Check if it is the first part of an IME input.
                    if ((character >= 0xD800) && (character <= 0xDBFF))
                        imeFirstWideChar = character;
                    else
                    {
                        // Check if it is the second part of an IME input.
                        if ((character >= 0xDC00) && (character <= 0xDFFF))
                        {
                            WindowEvent event(WindowEvent::Type::Text);
                            event.data.textData.utf = (imeFirstWideChar << 16) + character;
                            imeFirstWideChar = 0;
                            PushEvent(event);
                        }
                        else
                        {
                            WindowEvent event(WindowEvent::Type::Text);
                            event.data.textData.utf = character;
                            imeFirstWideChar = 0;
                            PushEvent(event);
                        }
                    }
                }
                break;
            }
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            {
                if (_enableKeyRepeat || ((HIWORD(lParam) & KF_REPEAT) == 0))
                {
                    WindowEvent event(WindowEvent::Type::KeyPressed);
                    event.data.keyData.alt = HIWORD(GetKeyState(VK_MENU)) != 0;
                    event.data.keyData.control = HIWORD(GetKeyState(VK_CONTROL)) != 0;
                    event.data.keyData.shift = HIWORD(GetKeyState(VK_SHIFT)) != 0;
                    event.data.keyData.system = HIWORD(GetKeyState(VK_LWIN)) || HIWORD(GetKeyState(VK_RWIN));
                    event.data.keyData.key = Keyboard::WinVirtualKeyToKeyCode(static_cast<int>(wParam), reinterpret_cast<void*>(lParam));
                    PushEvent(event);
                }
                break;
            }
            case WM_KEYUP:
            case WM_SYSKEYUP:
            {
                WindowEvent event(WindowEvent::Type::KeyReleased);
                event.data.keyData.alt = HIWORD(GetKeyState(VK_MENU)) != 0;
                event.data.keyData.control = HIWORD(GetKeyState(VK_CONTROL)) != 0;
                event.data.keyData.shift = HIWORD(GetKeyState(VK_SHIFT)) != 0;
                event.data.keyData.system = HIWORD(GetKeyState(VK_LWIN)) || HIWORD(GetKeyState(VK_RWIN));
                event.data.keyData.key = Keyboard::WinVirtualKeyToKeyCode(static_cast<int>(wParam), reinterpret_cast<void*>(lParam));
                PushEvent(event);
                break;
            }
            case WM_MOUSEWHEEL:
            {
                POINT position;
                position.x = static_cast<int16_t>(LOWORD(lParam));
                position.y = static_cast<int16_t>(HIWORD(lParam));
                ::ScreenToClient(reinterpret_cast<HWND>(_hWindow), &position);

                auto delta = static_cast<int16_t>(HIWORD(wParam));

                WindowEvent event(WindowEvent::Type::MouseWheel);
                event.data.mouseWheelData.delta = static_cast<float>(delta) / 120.f;
                event.data.mouseWheelData.x = position.x;
                event.data.mouseWheelData.y = position.y;
                PushEvent(event);
                break;
            }
            case WM_LBUTTONDOWN:
            {
                WindowEvent event(WindowEvent::Type::MouseButtonPressed);
                event.data.mouseButtonData.button = Mouse::Button::Left;
                event.data.mouseButtonData.x = static_cast<int16_t>(LOWORD(lParam));
                event.data.mouseButtonData.y = static_cast<int16_t>(HIWORD(lParam));
                PushEvent(event);
                break;
            }
            case WM_LBUTTONUP:
            {
                WindowEvent event(WindowEvent::Type::MouseButtonReleased);
                event.data.mouseButtonData.button = Mouse::Button::Left;
                event.data.mouseButtonData.x = static_cast<int16_t>(LOWORD(lParam));
                event.data.mouseButtonData.y = static_cast<int16_t>(HIWORD(lParam));
                PushEvent(event);
                break;
            }
            case WM_RBUTTONDOWN:
            {
                WindowEvent event(WindowEvent::Type::MouseButtonPressed);
                event.data.mouseButtonData.button = Mouse::Button::Right;
                event.data.mouseButtonData.x = static_cast<int16_t>(LOWORD(lParam));
                event.data.mouseButtonData.y = static_cast<int16_t>(HIWORD(lParam));
                PushEvent(event);
                break;
            }
            case WM_RBUTTONUP:
            {
                WindowEvent event(WindowEvent::Type::MouseButtonReleased);
                event.data.mouseButtonData.button = Mouse::Button::Right;
                event.data.mouseButtonData.x = static_cast<int16_t>(LOWORD(lParam));
                event.data.mouseButtonData.y = static_cast<int16_t>(HIWORD(lParam));
                PushEvent(event);
                break;
            }
            case WM_MBUTTONDOWN:
            {
                WindowEvent event(WindowEvent::Type::MouseButtonPressed);
                event.data.mouseButtonData.button = Mouse::Button::Middle;
                event.data.mouseButtonData.x = static_cast<int16_t>(LOWORD(lParam));
                event.data.mouseButtonData.y = static_cast<int16_t>(HIWORD(lParam));
                PushEvent(event);
                break;
            }
            case WM_MBUTTONUP:
            {
                WindowEvent event(WindowEvent::Type::MouseButtonReleased);
                event.data.mouseButtonData.button = Mouse::Button::Middle;
                event.data.mouseButtonData.x = static_cast<int16_t>(LOWORD(lParam));
                event.data.mouseButtonData.y = static_cast<int16_t>(HIWORD(lParam));
                PushEvent(event);
                break;
            }
            case WM_XBUTTONDOWN:
            {
                WindowEvent event(WindowEvent::Type::MouseButtonPressed);
                event.data.mouseButtonData.button = HIWORD(wParam) == XBUTTON1 ? Mouse::Button::Addition1 : Mouse::Button::Addition2;
                event.data.mouseButtonData.x = static_cast<int16_t>(LOWORD(lParam));
                event.data.mouseButtonData.y = static_cast<int16_t>(HIWORD(lParam));
                PushEvent(event);
                break;
            }
            case WM_XBUTTONUP:
            {
                WindowEvent event(WindowEvent::Type::MouseButtonReleased);
                event.data.mouseButtonData.button = HIWORD(wParam) == XBUTTON1 ? Mouse::Button::Addition1 : Mouse::Button::Addition2;
                event.data.mouseButtonData.x = static_cast<int16_t>(LOWORD(lParam));
                event.data.mouseButtonData.y = static_cast<int16_t>(HIWORD(lParam));
                PushEvent(event);
                break;
            }
            case WM_MOUSEMOVE:
            {
                HWND hWnd = reinterpret_cast<HWND>(_hWindow);
                int x = static_cast<int16_t>(LOWORD(lParam));
                int y = static_cast<int16_t>(HIWORD(lParam));

                RECT area;
                ::GetClientRect(hWnd, &area);

                // Capture the mouse in case the user wants to drag it outside
                if ((wParam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2)) == 0)
                {
                    if (::GetCapture() == hWnd)
                        ::ReleaseCapture();
                }
                else if (::GetCapture() != hWnd)
                {
                    ::SetCapture(hWnd);
                }

                // Mouse is out of window
                if ((x < area.left) || (x > area.right) || (y < area.top) || (y > area.bottom))
                {
                    if (_mouseInsideWindow)
                    {
                        _mouseInsideWindow = false;
                        WindowEvent event(WindowEvent::Type::MouseLeave);
                        PushEvent(event);
                    }
                }
                else
                {
                    if (!_mouseInsideWindow)
                    {
                        _mouseInsideWindow = true;
                        WindowEvent event(WindowEvent::Type::MouseEnter);
                        PushEvent(event);
                    }
                }

                WindowEvent event(WindowEvent::Type::MouseMoved);
                event.data.mouseMoveData.x = x;
                event.data.mouseMoveData.y = y;
                PushEvent(event);
                break;
            }
        }

    }
}

#endif