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

    std::vector<VkLayerProperties> supportedLayers;
    std::vector<const char*> validationLayers;

    {
        // Get all layers
        uint32_t vkLayerPropertiesCount = 0;
        if (::vkEnumerateInstanceLayerProperties(&vkLayerPropertiesCount, nullptr) != VK_SUCCESS)
            throw std::runtime_error("failed to enumerate layer properties!");

        supportedLayers.resize(vkLayerPropertiesCount);

        if (::vkEnumerateInstanceLayerProperties(&vkLayerPropertiesCount, &supportedLayers[0]) != VK_SUCCESS)
            throw std::runtime_error("failed to enumerate layer properties!");

        // Add validation layers
        const char* VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";
        for (std::size_t i = 0; i < supportedLayers.size(); i++)
        {
            if (std::strcmp(supportedLayers[i].layerName, VALIDATION_LAYER_NAME) == 0)
                validationLayers.push_back(VALIDATION_LAYER_NAME);
        }
    }

    std::vector<const char*> enableExtensions = NWA::Window::Vulkan::GetRequiredInstanceExtensions();
    enableExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

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
        createInfo.enabledExtensionCount = enableExtensions.size();
        createInfo.ppEnabledExtensionNames = enableExtensions.data();

        auto createInstanceRet = ::vkCreateInstance(&createInfo, nullptr, &vkInstance);
        if (createInstanceRet != VK_SUCCESS)
            throw std::runtime_error("failed to create instance!");
    }

#pragma endregion

#pragma region [Debug messager]

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