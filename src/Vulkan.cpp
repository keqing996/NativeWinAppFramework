
#include "NativeWinApp/Vulkan.h"

namespace NWA
{
    class VulkanDllLoader
    {
    public:
        VulkanDllLoader(const VulkanDllLoader&) = delete;

        ~VulkanDllLoader()
        {
            if (_loaded)
            {
                ::FreeLibrary(_hVulkanDll);
                _hVulkanDll = nullptr;
                _loaded = false;
            }
        }

        static VulkanDllLoader& Get()
        {
            static VulkanDllLoader sLoader;

            if (!sLoader._loaded)
                sLoader.Load();

            return sLoader;
        }

    private:
        VulkanDllLoader() = default;

        bool Load()
        {
            if (_hVulkanDll != nullptr)
                return true;

            _hVulkanDll = ::LoadLibraryA("vulkan-1.dll");

            if (_hVulkanDll == nullptr)
                return false;

            _vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(::GetProcAddress(_hVulkanDll, "vkGetInstanceProcAddr"));
            if (_vkGetInstanceProcAddr == nullptr)
            {
                ::FreeLibrary(_hVulkanDll);
                _hVulkanDll = nullptr;
                return false;
            }

            _loaded = true;

            return true;
        }

    public:
        PFN_vkGetInstanceProcAddr VkGetInstanceProcAddr() const
        {
            return _vkGetInstanceProcAddr;
        }

    private:
        bool _loaded = false;
        HMODULE _hVulkanDll = nullptr;
        PFN_vkGetInstanceProcAddr _vkGetInstanceProcAddr = nullptr;
    };

    const std::vector<const char*>& Vulkan::GetRequiredInstanceExtensions()
    {
        static std::vector<const char*> extensions;

        if (extensions.empty())
        {
            extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
            extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
        }

        return extensions;
    }

    bool Vulkan::CreateVulkanSurface(const VkInstance& instance, const Window& window, VkSurfaceKHR& surface, const VkAllocationCallbacks* allocator)
    {
        const auto vkProcLoader = VulkanDllLoader::Get().VkGetInstanceProcAddr();
        if (vkProcLoader == nullptr)
            return false;

        const auto vkCreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkProcLoader(instance, "vkCreateWin32SurfaceKHR"));
        if (vkCreateWin32SurfaceKHR == nullptr)
            return false;

        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = VkWin32SurfaceCreateInfoKHR();
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.hinstance = ::GetModuleHandleA(nullptr);
        surfaceCreateInfo.hwnd = static_cast<HWND>(window.GetSystemHandle());

        return vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, allocator, &surface) == VK_SUCCESS;
    }
}
