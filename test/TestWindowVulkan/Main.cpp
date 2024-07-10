#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include "NativeWinApp/Window.h"
#include "NativeWinApp/Vulkan.h"
#include "shader.vert.h"
#include "shader.frag.h"

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, void*);

VkResult CreateDebugUtilsMessengerEXT(VkInstance, const VkDebugUtilsMessengerCreateInfoEXT*, const VkAllocationCallbacks*, VkDebugUtilsMessengerEXT*);
void DestroyDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks*);

int main()
{
    int windowWidth = 800;
    int windowHeight = 800;

    NWA::Window window(windowWidth, windowHeight, "TestVulkan");

#pragma region [Setup layers & extensions]

    std::vector<const char*> activeLayers;
    std::vector<const char*> instanceLevelExtension;
    std::vector<const char*> deviceLevelExtension;

    {
        // Get all layers
        uint32_t vkLayerPropertiesCount = 0;
        ::vkEnumerateInstanceLayerProperties(&vkLayerPropertiesCount, nullptr);
        std::vector<VkLayerProperties> supportedLayers(vkLayerPropertiesCount);
        ::vkEnumerateInstanceLayerProperties(&vkLayerPropertiesCount, supportedLayers.data());

        // Add validation layers
        const char* VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";
        for (std::size_t i = 0; i < supportedLayers.size(); i++)
        {
            if (std::strcmp(supportedLayers[i].layerName, VALIDATION_LAYER_NAME) == 0)
                activeLayers.push_back(VALIDATION_LAYER_NAME);
        }

        // Instance level extension
        auto windowRequiredExtensions = NWA::Vulkan::GetRequiredInstanceExtensions();
        instanceLevelExtension.insert(instanceLevelExtension.end(), windowRequiredExtensions.begin(), windowRequiredExtensions.end());
        instanceLevelExtension.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        // Device level extensions -> only need swap chain
        deviceLevelExtension.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

#pragma endregion

#pragma region [Vulkan instance]

    VkInstance vkInstance;

    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "TestVulkan";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = activeLayers.size();
        createInfo.ppEnabledLayerNames = activeLayers.data();
        createInfo.enabledExtensionCount = instanceLevelExtension.size();
        createInfo.ppEnabledExtensionNames = instanceLevelExtension.data();

        auto createInstanceRet = ::vkCreateInstance(&createInfo, nullptr, &vkInstance);
        if (createInstanceRet != VK_SUCCESS)
            throw std::runtime_error("failed to create instance!");
    }

#pragma endregion

#pragma region [Debug message]

    VkDebugUtilsMessengerEXT vkDebugUtilExtHandle;

    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;

        if (CreateDebugUtilsMessengerEXT(vkInstance, &createInfo, nullptr, &vkDebugUtilExtHandle) != VK_SUCCESS)
            throw std::runtime_error("failed to set up debug messenger!");
    }

#pragma endregion

#pragma region [Window surface]

    VkSurfaceKHR vkSurface;
    if (!NWA::Vulkan::CreateVulkanSurface(vkInstance, window, vkSurface, nullptr))
        throw std::runtime_error("failed to create window surface!");

#pragma endregion

#pragma region [Physical device]

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    {
        uint32_t deviceCount = 0;
        ::vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        ::vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

        for (const auto& device : devices)
        {
            VkPhysicalDeviceProperties deviceProperties;
            ::vkGetPhysicalDeviceProperties(device, &deviceProperties);

            uint32_t extensionCount = 0;
            ::vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> extensions(extensionCount);
            ::vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

            bool supportsSwapchain = false;
            for (auto& extension: extensions)
            {
                if (std::strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
                {
                    supportsSwapchain = true;
                    break;
                }
            }

            if (!supportsSwapchain)
                continue;

            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                physicalDevice = device;

            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                physicalDevice = device;
                break;
            }
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("failed to find a suitable GPU!");

    VkFormat depthFormat;

    {
        VkFormatProperties formatProperties {};
        ::vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_D24_UNORM_S8_UINT, &formatProperties);
        if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
        else
        {
            ::vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_D32_SFLOAT_S8_UINT, &formatProperties);

            if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
            else
            {
                ::vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_D32_SFLOAT, &formatProperties);

                if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                    depthFormat = VK_FORMAT_D32_SFLOAT;
                else
                    throw std::runtime_error("failed to find a suitable depth format!");
            }
        }
    }

#pragma endregion

#pragma region [Logic device]

    VkDevice logicDevice;
    VkQueue deviceQueue;

    {
        uint32_t queueFamilyCount = 0;
        ::vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        ::vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

        int queueFamilyIndex = -1;
        for (std::size_t i = 0; i < queueFamilyProperties.size(); i++)
        {
            // Support graphics
            bool supportGraphics = queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;

            // Support compute
            bool supportCompute = queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT;

            // Support surface
            VkBool32 supportPresentation = false;
            ::vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, vkSurface, &supportPresentation);

            // By vulkan spec, we can not assum that there will be one queue statisfy "graphics", "compute"
            // and "Presentation" at the same time. But we just make this assumption, it's fine.
            if (supportGraphics && supportCompute && supportPresentation)
            {
                queueFamilyIndex = static_cast<int>(i);
                break;
            }
        }

        if (queueFamilyIndex < 0)
            throw std::runtime_error("failed to queue families!");

        float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo deviceQueueCreateInfo {};
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.queueCount = 1;
        deviceQueueCreateInfo.queueFamilyIndex = static_cast<uint32_t>(queueFamilyIndex);
        deviceQueueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures physicalDeviceFeatures {};
        physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo deviceCreateInfo {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.enabledExtensionCount = deviceLevelExtension.size();
        deviceCreateInfo.ppEnabledExtensionNames = deviceLevelExtension.data();
        deviceCreateInfo.enabledLayerCount = activeLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = activeLayers.data();
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
        deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

        if (::vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicDevice) != VK_SUCCESS)
            throw std::runtime_error("failed to create logic device!");

        ::vkGetDeviceQueue(logicDevice, static_cast<uint32_t>(queueFamilyIndex), 0, &deviceQueue);
    }

#pragma endregion

#pragma region [Swapchain]

    VkSurfaceFormatKHR swapchainFormat;

    {
        // Foarmat -> find a format that supports RGBA
        uint32_t surfaceFormatsCount = 0;
        ::vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vkSurface, &surfaceFormatsCount, nullptr);
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatsCount);
        ::vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vkSurface, &surfaceFormatsCount, surfaceFormats.data());

        for (auto& [format, colorSpace]: surfaceFormats)
        {
            if ((format == VK_FORMAT_B8G8R8A8_UNORM) && (colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR))
            {
                swapchainFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
                swapchainFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                break;
            }
        }

        if (swapchainFormat.format == VK_FORMAT_UNDEFINED)
            throw std::runtime_error("failed to find suitable color format!");
    }

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

    {
        uint32_t presentModesCount;
        ::vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vkSurface, &presentModesCount, nullptr);
        std::vector<VkPresentModeKHR> presentModes;
        ::vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vkSurface, &presentModesCount, presentModes.data());

        for (auto mode: presentModes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)    // Prefer mail box mode
            {
                presentMode = mode;
                break;
            }
        }
    }

    VkSurfaceCapabilitiesKHR surfaceCapabilities {};

    {
        if (::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vkSurface, &surfaceCapabilities) != VK_SUCCESS)
            throw std::runtime_error("failed to query surface capabilities!");
    }

    VkExtent2D swapchainExtent;
    VkSwapchainKHR swapchain;

    {
        auto Clamp = [](uint32_t value, uint32_t min, uint32_t max) -> uint32_t
        {
            return (value <= min) ? min : ((value >= max) ? max : value);
        };

        swapchainExtent.width  = Clamp(window.GetSize().first, surfaceCapabilities.minImageExtent.width,  surfaceCapabilities.maxImageExtent.width);
        swapchainExtent.height = Clamp(window.GetSize().second, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

        uint32_t imageCount = Clamp(2, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

        VkSwapchainCreateInfoKHR swapchainCreateInfo {};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface = vkSurface;
        swapchainCreateInfo.minImageCount = imageCount;
        swapchainCreateInfo.imageFormat = swapchainFormat.format;
        swapchainCreateInfo.imageColorSpace = swapchainFormat.colorSpace;
        swapchainCreateInfo.imageExtent = swapchainExtent;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.presentMode = presentMode;
        swapchainCreateInfo.clipped = VK_TRUE;
        swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

        if (::vkCreateSwapchainKHR(logicDevice, &swapchainCreateInfo, nullptr, &swapchain) != VK_SUCCESS)
            throw std::runtime_error("failed to create swap chain!");
    }

#pragma endregion

#pragma region [Swapchain image view]

    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;

    {
        uint32_t imageCount;
        ::vkGetSwapchainImagesKHR(logicDevice, swapchain, &imageCount, nullptr);
        swapchainImages.resize(imageCount);
        ::vkGetSwapchainImagesKHR(logicDevice, swapchain, &imageCount, swapchainImages.data());

        swapchainImageViews.resize(imageCount);

        VkImageViewCreateInfo imageViewCreateInfo {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = swapchainFormat.format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        for (std::size_t i = 0; i < swapchainImages.size(); i++)
        {
            imageViewCreateInfo.image = swapchainImages[i];

            if (::vkCreateImageView(logicDevice, &imageViewCreateInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to create swapchain image view!");
        }
    }

#pragma endregion

    // Main loop
    while (true)
    {
        window.EventLoop();

        if (std::ranges::any_of(window.PopAllEvent(), [](const NWA::WindowEvent& event) -> bool { return event.type == NWA::WindowEvent::Type::Close; }))
            break;
    }

    // Clearup

    for (auto imageView : swapchainImageViews)
        ::vkDestroyImageView(logicDevice, imageView, nullptr);

    ::vkDestroySwapchainKHR(logicDevice, swapchain, nullptr);

    ::vkDestroyDevice(logicDevice, nullptr);

    DestroyDebugUtilsMessengerEXT(vkInstance, vkDebugUtilExtHandle, nullptr);

    ::vkDestroyInstance(vkInstance, nullptr);

    return 0;

}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)::vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);

    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)::vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, debugMessenger, pAllocator);
}