#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include "NativeWinApp/Window.h"

bool CheckLayerSupport(const std::string& layerName);

int main()
{
    int windowWidth = 800;
    int windowHeight = 800;

    NWA::Window window(windowWidth, windowHeight, "TestVulkan");

#pragma region [Setup layers & extensions]

    std::vector<VkLayerProperties> vkLayerProperties;

    {
        uint32_t vkLayerPropertiesCount = 0;
        if (::vkEnumerateInstanceLayerProperties(&vkLayerPropertiesCount, nullptr) != VK_SUCCESS)
            throw std::runtime_error("failed to enumerate layer properties!");

        vkLayerProperties.resize(vkLayerPropertiesCount);

        if (::vkEnumerateInstanceLayerProperties(&vkLayerPropertiesCount, &vkLayerProperties[0]) != VK_SUCCESS)
            throw std::runtime_error("failed to enumerate layer properties!");
    }

    std::vector<const char*> vkExtensions = NWA::Window::Vulkan::GetRequiredInstanceExtensions();

    {
        vkExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }


#pragma endregion

#pragma region [Vulkan Instance]

    VkInstance vkInstance;

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

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    createInfo.enabledLayerCount = 0;

    auto createInstanceRet = ::vkCreateInstance(&createInfo, nullptr, &vkInstance);
    if (createInstanceRet != VK_SUCCESS)
        throw std::runtime_error("failed to create instance!");


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

bool CheckLayerSupport(const std::string& layerName)
{
    static bool layerPropertiesInit = false;
    static uint32_t layerCount = 0;
    static std::vector<VkLayerProperties> availableLayers;

    if (!layerPropertiesInit)
    {
        ::vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        availableLayers.resize(layerCount);
        ::vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        layerPropertiesInit = true;
    }

    for (const auto& layerProperties : availableLayers)
    {
        if (::strcmp(layerName.c_str(), layerProperties.layerName) == 0)
            return true;
    }

    return false;
}