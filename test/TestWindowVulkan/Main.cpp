#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include "NativeWinApp/Window.h"

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT, const VkDebugUtilsMessengerCallbackDataEXT*, void*);

VkResult CreateDebugUtilsMessengerEXT(VkInstance, const VkDebugUtilsMessengerCreateInfoEXT*, const VkAllocationCallbacks*, VkDebugUtilsMessengerEXT*);
void DestroyDebugUtilsMessengerEXT(VkInstance, VkDebugUtilsMessengerEXT, const VkAllocationCallbacks*);

int main()
{
    int windowWidth = 800;
    int windowHeight = 800;

    NWA::Window window(windowWidth, windowHeight, "TestVulkan");

#pragma region [Setup layers & extensions]

    std::vector<const char*> validationLayers;

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
                validationLayers.push_back(VALIDATION_LAYER_NAME);
        }
    }

    std::vector<const char*> platformExtension = NWA::Window::Vulkan::GetRequiredInstanceExtensions();
    platformExtension.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

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
        createInfo.enabledLayerCount = validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();
        createInfo.enabledExtensionCount = platformExtension.size();
        createInfo.ppEnabledExtensionNames = platformExtension.data();

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
    int queueFamilyIndex = -1;
    VkSurfaceKHR surface;
    const char* deviceExtensions[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    {
        uint32_t queueFamilyCount = 0;
        ::vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        ::vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

        for (std::size_t i = 0; i < queueFamilyProperties.size(); i++)
        {
            VkBool32 surfaceSupported = VK_FALSE;

            ::vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &surfaceSupported);

            if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (surfaceSupported == VK_TRUE))
            {
                queueFamilyIndex = static_cast<int>(i);
                break;
            }
        }

        if (queueFamilyIndex < 0)
            throw std::runtime_error("failed to queue families!");

        float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo deviceQueueCreateInfo = VkDeviceQueueCreateInfo();
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.queueCount = 1;
        deviceQueueCreateInfo.queueFamilyIndex = static_cast<uint32_t>(queueFamilyIndex);
        deviceQueueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures physicalDeviceFeatures = VkPhysicalDeviceFeatures();
        physicalDeviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo deviceCreateInfo = VkDeviceCreateInfo();
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.enabledExtensionCount = 1;
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
        deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;

        if (::vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicDevice) != VK_SUCCESS)
            throw std::runtime_error("failed to create logic device!");

        ::vkGetDeviceQueue(logicDevice, static_cast<uint32_t>(queueFamilyIndex), 0, &deviceQueue);
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