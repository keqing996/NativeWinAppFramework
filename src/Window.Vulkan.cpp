#include "NativeWinApp/WindowsInclude.h"
#include "NativeWinApp/Window.h"
#include "WinVulkan.h"

namespace NWA
{
    const std::vector<const char*>& Window::Vulkan::GetRequiredInstanceExtensions()
    {
        static std::vector<const char*> extensions;

        if (extensions.empty())
        {
            extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
            extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        }

        return extensions;
    }
}