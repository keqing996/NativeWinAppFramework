#pragma once

#include "Window.h"


#define VK_USE_PLATFORM_WIN32_KHR // for windows platform
#define VK_NO_PROTOTYPES // for static link
#include <vulkan/vulkan.h>

namespace NWA
{
    class Vulkan
    {
    public:
        static const std::vector<const char*>& GetRequiredInstanceExtensions();
        static bool CreateVulkanSurface(const VkInstance& instance, const Window& window, VkSurfaceKHR& surface, const VkAllocationCallbacks* allocator);
    };
}