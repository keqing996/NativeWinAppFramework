#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include "NativeWinApp/Window.h"

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
        const char* VALIDATION_LAYER_NAME = "VK_LAYER_LUNARG_standard_validation";
        const char* MONITOR_LAYER_NAME = "VK_LAYER_LUNARG_monitor";
        for (std::size_t i = 0; i < supportedLayers.size(); i++)
        {
            if (std::strcmp(supportedLayers[i].layerName, VALIDATION_LAYER_NAME) == 0)
                validationLayers.push_back(VALIDATION_LAYER_NAME);
            else if (std::strcmp(supportedLayers[i].layerName, MONITOR_LAYER_NAME) == 0)
                validationLayers.push_back(MONITOR_LAYER_NAME);
        }
    }

    std::vector<const char*> enableExtensions = NWA::Window::Vulkan::GetRequiredInstanceExtensions();
    enableExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);


#pragma endregion

#pragma region [Vulkan Instance]

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

    // Main loop
    while (true)
    {
        window.EventLoop();

        if (std::ranges::any_of(window.PopAllEvent(), [](const NWA::WindowEvent& event) -> bool { return event.type == NWA::WindowEvent::Type::Close; }))
            break;
    }


    return 0;

}