#pragma once

#include "WindowEvent.h"
#include <cstdint>
#include <string>
#include <queue>
#include <optional>
#include <functional>

namespace NWA
{
    enum class WindowStyle: int
    {
        None = 0,
        HaveTitleBar = 1 << 0,
        HaveResize = 1 << 1,
        HaveClose = 1 << 2,
    };

    inline int WindowStyleDefault = static_cast<int>(WindowStyle::HaveTitleBar) | static_cast<int>(WindowStyle::HaveResize) | static_cast<int>(WindowStyle::HaveClose);
    inline int WindowStyleNoResize = static_cast<int>(WindowStyle::HaveTitleBar) | static_cast<int>(WindowStyle::HaveClose);
    inline int WindowStyleNoClose = static_cast<int>(WindowStyle::HaveTitleBar) | static_cast<int>(WindowStyle::HaveResize);

    class Window
    {
    public:
        using WindowHandle = void*;
        using IconHandle = void*;
        using CurosrHandle = void*;
        using DeviceContextHandle = void*;
        using GLContextHandle = void*;

    public:
        Window(int width, int height, const std::string& title, int style = WindowStyleDefault);
        ~Window();

    public:
        auto EventLoop() -> void;
        auto CreateOpenGLContext() -> void;
        auto SwapBuffer() const -> void;
        auto WindowEventProcess(uint32_t message, void* wpara, void* lpara) -> void;
        auto SetWindowEventProcessFunction(const std::function<bool(void*, uint32_t, void*, void*)>& f) -> void;
        auto ClearWindowEventProcessFunction() -> void;
        auto HasEvent() const -> bool;
        auto PopEvent(WindowEvent& outEvent) -> bool;
        auto PopAllEvent() -> std::vector<WindowEvent>;

        auto GetSize() const -> std::pair<int, int>;
        auto SetSize(int width, int height) -> void;

        auto GetPosition() const -> std::pair<int, int>;
        auto SetPosition(int x, int y) -> void;

        auto GetSystemHandle() const -> void*;

        auto SetIcon(unsigned int width, unsigned int height, const std::byte* pixels) -> void;
        auto SetIcon(int iconResId) -> void;

        auto SetTitle(const std::string& title) -> void;

        auto SetWindowVisible(bool show) -> void;

        auto GetCursorVisible() const -> bool;
        auto SetCursorVisible(bool show) -> void;

        auto GetCursorCapture() const -> bool;
        auto SetCursorCapture(bool capture) -> void;

        auto GetKeyRepeated() const -> bool;
        auto SetKeyRepeated(bool repeated) -> void;

    private:
        auto WindowEventProcessInternal(uint32_t message, void* wpara, void* lpara) -> void;
        auto PushEvent(const WindowEvent& event) -> void;
        auto CaptureCursorInternal(bool doCapture) -> void;

    private:
        // Window handle
        WindowHandle _hWindow;
        DeviceContextHandle _hDeviceHandle;

        // State
        std::pair<int, int> _windowSize;
        bool _enableKeyRepeat;
        bool _cursorVisible;
        bool _cursorCapture;
        bool _mouseInsideWindow;

        // Resource
        IconHandle _hIcon;
        CurosrHandle _hCursor;

        // Event
        std::queue<WindowEvent> _eventQueue;

        // Additional handler
        std::function<bool(void*, uint32_t, void*, void*)> _winEventProcess;

        // OpenGL
        GLContextHandle _hGLContext;

    private:
        static void RegisterWindowClass();
        static void UnRegisterWindowClass();

    private:
        inline static int _sGlobalWindowsCount = 0;
        inline static const wchar_t* _sWindowRegisterName = L"InfraWindow";

    public:
        class Vulkan
        {
        public:
            static const std::vector<const char*>& GetRequiredInstanceExtensions();
        };
    };
}