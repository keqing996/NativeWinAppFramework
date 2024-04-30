#include "../../include/NativeWinApp/Window.h"
#include "../../include/NativeWinApp/WindowsDefine.h"

#if PLATFORM_WINDOWS

#pragma comment(lib, "opengl32.lib")

#include "../../ScopeGuard.hpp"
#include "../../String.hpp"
#include "../../Logger.hpp"

#include "../../Windows/Glad/Gl.h"

namespace Infra
{
    class Support
    {
    public:
        Support() = delete;

    public:
        static bool FixProcessDpi()
        {
            // Try SetProcessDpiAwareness first
            if (FixProcessDpiBySetProcessDpiAwareness())
                return true;

            // Fall back to SetProcessDPIAware
            if (FixProcessDpiBySetProcessDPIAware())
                return true;

            return false;
        }

        static LRESULT CALLBACK DefaultWndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
        {
            if (message == WM_CREATE)
            {
                LONG_PTR window = reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
                ::SetWindowLongPtrW(handle, GWLP_USERDATA, window);
            }

            Window* pWindow = handle ? reinterpret_cast<Window*>(::GetWindowLongPtrW(handle, GWLP_USERDATA)) : nullptr;

            if (pWindow)
            {
                pWindow->WindowEventProcess(message, reinterpret_cast<void*>(wParam), reinterpret_cast<void*>(lParam));
            }

            // If drop WM_CLOSE into DefWindowProcW, window will destroy.
            // Push message to event queue, and throw it to user.
            if (message == WM_CLOSE)
                return 0;

            // Hack the menu system command, so that pressing ALT or F10 doesn't steal the focus
            if ((message == WM_SYSCOMMAND) && (wParam == SC_KEYMENU))
                return 0;

            return ::DefWindowProcW(handle, message, wParam, lParam);
        }

        static std::pair<int, int> CalculateAdjustWindowSize(int width, int height, DWORD dwStyle)
        {
            RECT rectangle = {0, 0, width, height};

            ::AdjustWindowRect(&rectangle, dwStyle, false);

            return { rectangle.right - rectangle.left, rectangle.bottom - rectangle.top };
        }

    private:
        static bool FixProcessDpiBySetProcessDpiAwareness()
        {
            HINSTANCE pShcodeDll = ::LoadLibraryW(L"Shcore.dll");

            if (pShcodeDll == nullptr)
                return false;

            ScopeGuard pShcodeDllReleaseGuard = [&pShcodeDll]()
            {
                ::FreeLibrary(pShcodeDll);
            };

            void* pFuncSetProcessDpiAwareness = reinterpret_cast<void*>(GetProcAddress(pShcodeDll, "SetProcessDpiAwareness"));
            if (pFuncSetProcessDpiAwareness == nullptr)
                return false;

            enum ProcessDpiAwareness
            {
                ProcessDpiUnaware         = 0,
                ProcessSystemDpiAware     = 1,
                ProcessPerMonitorDpiAware = 2
            };

            using SetProcessDpiAwarenessFuncType = HRESULT (WINAPI*)(ProcessDpiAwareness);

            auto setProcessDpiAwarenessFunc = reinterpret_cast<SetProcessDpiAwarenessFuncType>(pFuncSetProcessDpiAwareness);
            if (setProcessDpiAwarenessFunc(ProcessPerMonitorDpiAware) == E_INVALIDARG)
                return false;

            return true;
        }

        static bool FixProcessDpiBySetProcessDPIAware()
        {
            HINSTANCE pUser32Dll = ::LoadLibraryW(L"user32.dll");

            if (pUser32Dll == nullptr)
                return false;

            ScopeGuard pShcodeDllReleaseGuard = [&pUser32Dll]()
            {
                ::FreeLibrary(pUser32Dll);
            };

            void* pFuncSetProcessDPIAware = reinterpret_cast<void*>(GetProcAddress(pUser32Dll, "SetProcessDPIAware"));
            if (pFuncSetProcessDPIAware == nullptr)
                return false;

            using SetProcessDPIAwareFuncType = BOOL (WINAPI*)(void);

            auto setProcessDpiAwareFunc = reinterpret_cast<SetProcessDPIAwareFuncType>(pFuncSetProcessDPIAware);
            if (!setProcessDpiAwareFunc())
                return false;

            return true;
        }
    };

    Window::Window(int width, int height, const std::string& title, int style)
        : _hWindow(nullptr)
        , _hDeviceHandle(nullptr)
        , _windowSize({width, height})
        , _enableKeyRepeat(true)
        , _cursorVisible(true)
        , _cursorCapture(false)
        , _mouseInsideWindow(false)
        , _hIcon(nullptr)
        , _hCursor(::LoadCursor(nullptr, IDC_ARROW))
        , _hGLContext(nullptr)
    {
        // Fix dpi
        Support::FixProcessDpi();

        // Register window
        if (_sGlobalWindowsCount == 0)
            RegisterWindowClass();

        // Get create style
        DWORD win32Style = WS_VISIBLE;
        if (style == (int)WindowStyle::None)
            win32Style |= WS_POPUP;
        else
        {
            if (style & (int)WindowStyle::HaveTitleBar)
                win32Style |= WS_CAPTION | WS_MINIMIZEBOX;
            if (style & (int)WindowStyle::HaveResize)
                win32Style |= WS_THICKFRAME | WS_MAXIMIZEBOX;
            if (style & (int)WindowStyle::HaveClose)
                win32Style |= WS_SYSMENU;
        }

        // Calculate create position: left & top
        HDC screenDC = ::GetDC(nullptr);
        int left = (::GetDeviceCaps(screenDC, HORZRES) - width) / 2;
        int top = (::GetDeviceCaps(screenDC, VERTRES) - height) / 2;
        ::ReleaseDC(nullptr, screenDC);

        // Adjust create size
        auto [adjustWidth, adjustHeight] = Support::CalculateAdjustWindowSize(width, height, win32Style);

        // Create window
        auto titleInWideStr = String::StringToWideString(title);
        const wchar_t* titleWide = titleInWideStr.has_value() ? titleInWideStr->c_str() : L"Default Title";
        _hWindow = ::CreateWindowW(
                _sWindowRegisterName,
                titleWide,
                win32Style,
                left,
                top,
                adjustWidth,
                adjustHeight,
                nullptr,
                nullptr,
                ::GetModuleHandleW(nullptr),
                this);

        if (!_hWindow)
            return;

        // Get device context
        _hDeviceHandle = ::GetDC(reinterpret_cast<HWND>(_hWindow));

        // Global counting
        _sGlobalWindowsCount++;

        // Set size again after window creation to avoid some bug.
        SetSize(width, height);
    }

    Window::~Window()
    {
        SetCursorVisible(true);
        ::ReleaseCapture();

        // Release openGL
        if (_hGLContext)
        {
            ::wglMakeCurrent(reinterpret_cast<HDC>(_hDeviceHandle), nullptr);
            ::wglDeleteContext(reinterpret_cast<HGLRC>(_hGLContext));
        }

        if (_hDeviceHandle)
            ::ReleaseDC(reinterpret_cast<HWND>(_hWindow), reinterpret_cast<HDC>(_hDeviceHandle));

        // Icon
        if (_hIcon != nullptr)
            ::DestroyIcon(reinterpret_cast<HICON>(_hIcon));

        // Destroy window
        ::DestroyWindow(reinterpret_cast<HWND>(_hWindow));

        // Global counting
        _sGlobalWindowsCount--;

        if (_sGlobalWindowsCount == 0)
            UnRegisterWindowClass();
    }

    void Window::RegisterWindowClass()
    {
        WNDCLASSW windowClass;
        windowClass.style         = 0;
        windowClass.lpfnWndProc   = &Support::DefaultWndProc;
        windowClass.cbClsExtra    = 0;
        windowClass.cbWndExtra    = 0;
        windowClass.hInstance     = ::GetModuleHandleW(nullptr);
        windowClass.hIcon         = nullptr;
        windowClass.hCursor       = nullptr;
        windowClass.hbrBackground = nullptr;
        windowClass.lpszMenuName  = nullptr;
        windowClass.lpszClassName = _sWindowRegisterName;
        ::RegisterClassW(&windowClass);
    }

    void Window::UnRegisterWindowClass()
    {
        ::UnregisterClassW(_sWindowRegisterName, GetModuleHandleW(nullptr));
    }

    void Window::SetSize(int width, int height)
    {
        HWND hWnd = reinterpret_cast<HWND>(_hWindow);
        DWORD dwStyle = static_cast<DWORD>(::GetWindowLongPtrW(hWnd, GWL_STYLE));
        auto [adjustWidth, adjustHeight] = Support::CalculateAdjustWindowSize(width, height, dwStyle);
        ::SetWindowPos(hWnd, nullptr, 0, 0, adjustWidth, adjustHeight, SWP_NOMOVE | SWP_NOZORDER);
    }

    auto Window::GetSystemHandle() -> void*
    {
        return _hWindow;
    }

    auto Window::SetIcon(unsigned int width, unsigned int height, const std::byte* pixels) -> void
    {
        if (_hIcon != nullptr)
            ::DestroyIcon(reinterpret_cast<HICON>(_hIcon));

        _hIcon = nullptr;

        // Change RGBA to BGRA
        std::vector<std::byte> iconPixels(width * height * 4);
        for (std::size_t i = 0; i < iconPixels.size() / 4; ++i)
        {
            iconPixels[i * 4 + 0] = pixels[i * 4 + 2];
            iconPixels[i * 4 + 1] = pixels[i * 4 + 1];
            iconPixels[i * 4 + 2] = pixels[i * 4 + 0];
            iconPixels[i * 4 + 3] = pixels[i * 4 + 3];
        }

        _hIcon = ::CreateIcon(
                GetModuleHandleW(nullptr),
                static_cast<int>(width),
                static_cast<int>(height),
                1,
                32,
                nullptr,
                (unsigned char*)iconPixels.data());

        if (_hIcon != nullptr)
        {
            ::SendMessageW(
                    reinterpret_cast<HWND>(_hWindow),
                    WM_SETICON,
                    ICON_BIG,
                    reinterpret_cast<LPARAM>(_hIcon));

            ::SendMessageW(
                    reinterpret_cast<HWND>(_hWindow),
                    WM_SETICON,
                    ICON_SMALL,
                    reinterpret_cast<LPARAM>(_hIcon));
        }
    }

    auto Window::SetWindowVisible(bool show) -> void
    {
        ::ShowWindow(reinterpret_cast<HWND>(_hWindow), show ? SW_SHOW : SW_HIDE);
    }

    auto Window::SetCursorVisible(bool show) -> void
    {
        _cursorVisible = show;
        ::SetCursor(_cursorVisible ? reinterpret_cast<HCURSOR>(_hCursor) : nullptr);
    }

    auto Window::SetCursorCapture(bool capture) -> void
    {
        _cursorCapture = capture;
        CaptureCursorInternal(_cursorCapture);
    }

    auto Window::GetCursorVisible() -> bool
    {
        return _cursorVisible;
    }

    auto Window::GetCursorCapture() -> bool
    {
        return _cursorCapture;
    }

    auto Window::GetKeyRepeated() -> bool
    {
        return _enableKeyRepeat;
    }

    auto Window::SetKeyRepeated(bool repeated) -> void
    {
        _enableKeyRepeat = repeated;
    }

    auto Window::SetTitle(const std::string& title) -> void
    {
        auto titleInWideStr = String::StringToWideString(title);
        const wchar_t* titleWide = titleInWideStr.has_value() ? titleInWideStr->c_str() : L"Default Title";
        ::SetWindowTextW(reinterpret_cast<HWND>(_hWindow), titleWide);
    }

    auto Window::GetSize() -> std::pair<int, int>
    {
        RECT rect;
        ::GetClientRect(reinterpret_cast<HWND>(_hWindow), &rect);
        return { static_cast<int>(rect.right - rect.left), static_cast<int>(rect.bottom - rect.top) };
    }

    auto Window::GetPosition() -> std::pair<int, int>
    {
        RECT rect;
        ::GetWindowRect(reinterpret_cast<HWND>(_hWindow), &rect);

        return { static_cast<int>(rect.left), static_cast<int>(rect.top) };
    }

    auto Window::SetPosition(int x, int y) -> void
    {
        ::SetWindowPos(
                reinterpret_cast<HWND>(_hWindow),
                NULL,
                x,
                y,
                0,
                0,
                SWP_NOSIZE | SWP_NOZORDER);

        // Adjust cursor position
        if(_cursorCapture)
            SetCursorCapture(true);
    }

    auto Window::EventLoop() -> void
    {
        // Clear event queue
        while (!_eventQueue.empty())
            _eventQueue.pop();

        // Fetch new event
        MSG message;
        while (::PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&message);
            ::DispatchMessageW(&message);
        }
    }

    auto Window::HasEvent() -> bool
    {
        return !_eventQueue.empty();
    }

    auto Window::PopEvent() -> WindowEvent
    {
        if (_eventQueue.empty())
            return WindowEvent(WindowEvent::Type::None);

        WindowEvent result = _eventQueue.front();
        _eventQueue.pop();

        return result;
    }

    auto Window::PushEvent(const WindowEvent& event) -> void
    {
        _eventQueue.push(event);
    }

    auto Window::CaptureCursorInternal(bool doCapture) -> void
    {
        if (doCapture)
        {
            RECT rect;
            ::GetClientRect(reinterpret_cast<HWND>(_hWindow), &rect);
            ::MapWindowPoints(reinterpret_cast<HWND>(_hWindow), nullptr, reinterpret_cast<LPPOINT>(&rect), 2);
            ::ClipCursor(&rect);
        }
        else
        {
            ::ClipCursor(nullptr);
        }
    }

    auto Window::SetWindowEventProcessFunction(const std::function<void(uint32_t, void*, void*)>& f) -> void
    {
        _winEventProcess = f;
    }

    auto Window::ClearWindowEventProcessFunction() -> void
    {
        _winEventProcess = nullptr;
    }

    // https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)
    auto Window::CreateOpenGLContext() -> void
    {
        HDC hDeviceHandle = reinterpret_cast<HDC>(_hDeviceHandle);

        ::gladLoaderLoadGL();

        PIXELFORMATDESCRIPTOR pfd =
        {
                sizeof(PIXELFORMATDESCRIPTOR),
                1,
                PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
                PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
                32,                   // Color depth of the framebuffer.
                0, 0, 0, 0, 0, 0,
                0,
                0,
                0,
                0, 0, 0, 0,
                24,                   // Number of bits for the depth buffer
                8,                    // Number of bits for the stencil buffer
                0,                    // Number of Aux buffers in the framebuffer.
                PFD_MAIN_PLANE,
                0,
                0, 0, 0
        };

        int letWindowsChooseThisPixelFormat;
        letWindowsChooseThisPixelFormat = ::ChoosePixelFormat(hDeviceHandle, &pfd);
        ::SetPixelFormat(hDeviceHandle, letWindowsChooseThisPixelFormat, &pfd);

        _hGLContext = ::wglCreateContext(hDeviceHandle);
        ::wglMakeCurrent(hDeviceHandle, reinterpret_cast<HGLRC>(_hGLContext));
    }

    auto Window::SwapBuffer() -> void
    {
        ::SwapBuffers(reinterpret_cast<HDC>(_hDeviceHandle));
    }

}

#endif