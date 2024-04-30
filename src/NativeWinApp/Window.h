#pragma once

#include "../PlatformDefine.hpp"

#if PLATFORM_WINDOWS

#include "WindowEvent.h"
#include <cstdint>
#include <string>
#include <queue>
#include <optional>
#include <functional>

namespace Infra
{
    enum class WindowStyle: int
    {
        None = 0,
        HaveTitleBar = 1 << 0,
        HaveResize = 1 << 1,
        HaveClose = 1 << 2,
        Default = HaveTitleBar | HaveResize | HaveClose
    };

    class Window
    {
    public:
        using WindowHandle = void*;
        using IconHandle = void*;
        using CurosrHandle = void*;
        using DeviceContextHandle = void*;
        using GLContextHandle = void*;

    public:
        Window(int width, int height, const std::string& title, int style = (int)WindowStyle::Default);
        ~Window();

    public:
        auto EventLoop() -> void;
        auto CreateOpenGLContext() -> void;
        auto SwapBuffer() -> void;
        auto WindowEventProcess(uint32_t message, void* wpara, void* lpara) -> void;
        auto SetWindowEventProcessFunction(const std::function<void(uint32_t, void*, void*)>& f) -> void;
        auto ClearWindowEventProcessFunction() -> void;
        auto HasEvent() -> bool;
        auto PopEvent() -> WindowEvent;

        auto GetSize() -> std::pair<int, int>;
        auto SetSize(int width, int height) -> void;

        auto GetPosition() -> std::pair<int, int>;
        auto SetPosition(int x, int y) -> void;

        auto GetSystemHandle() -> void*;

        auto SetIcon(unsigned int width, unsigned int height, const std::byte* pixels) -> void;

        auto SetTitle(const std::string& title) -> void;

        auto SetWindowVisible(bool show) -> void;

        auto GetCursorVisible() -> bool;
        auto SetCursorVisible(bool show) -> void;

        auto GetCursorCapture() -> bool;
        auto SetCursorCapture(bool capture) -> void;

        auto GetKeyRepeated() -> bool;
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
        std::function<void(uint32_t, void*, void*)> _winEventProcess;

        // OpenGL
        GLContextHandle _hGLContext;

    private:
        static void RegisterWindowClass();
        static void UnRegisterWindowClass();

    private:
        inline static int _sGlobalWindowsCount = 0;
        inline static const wchar_t* _sWindowRegisterName = L"InfraWindow";


    };
}

#endif