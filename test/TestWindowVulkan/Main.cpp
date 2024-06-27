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

#pragma region [Create vulkan instance]

    static const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

    // CREATE VULKAN INSTANCE
    // The instance is the connection between application and the Vulkan library
    // and creating it involves specifying some details about your application to the driver.
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

    VkInstance vkInstance;
    if (::vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan instance!");

#pragma endregion

#pragma region [Create physical device]

    VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

    if (deviceCount == 0)
        throw std::runtime_error("No Vulkan-compatible GPU found!");

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        bool suitable = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
            && deviceFeatures.geometryShader;

        if (suitable)
        {
            vkPhysicalDevice = device;
            break;
        }
    }

    if (vkPhysicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("Failed to find a suitable GPU!");

#pragma endregion

#pragma region [Create logic device]

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());

    uint32_t graphicsFamilyIndex;
    for (int i = 0; i < queueFamilies.size(); i++)
    {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            graphicsFamilyIndex = static_cast<uint32_t>(i);
    }

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsFamilyIndex;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo logicCreateInfo{};
    logicCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    logicCreateInfo.queueCreateInfoCount = 1;
    logicCreateInfo.pEnabledFeatures = &deviceFeatures;

    VkDevice vkLogicDevice;
    if (vkCreateDevice(vkPhysicalDevice, &logicCreateInfo, nullptr, &vkLogicDevice) != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device!");

    VkQueue graphicsQueue;
    vkGetDeviceQueue(vkLogicDevice, graphicsFamilyIndex, 0, &graphicsQueue);

#pragma endregion

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

#pragma region [Create render pass]

    VkFormat swapChainImageFormat;
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkRenderPass renderPass;
    if (vkCreateRenderPass(vkLogicDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        throw std::runtime_error("Failed to create render pass!");

#pragma endregion

#pragma region [Create graphic pass]

#pragma endregion

#pragma region [Create frame buffers]

    std::vector<VkFramebuffer> swapChainFramebuffers;
    std::vector<VkImageView> swapChainImageViews;

    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = { swapChainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = windowWidth;
        framebufferInfo.height = windowHeight;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(vkLogicDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("Failed to create framebuffer!");
    }

#pragma endregion

#pragma region [Create command pool]

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = graphicsFamilyIndex;
    poolInfo.flags = 0; // Optional (VK_COMMAND_POOL_CREATE_TRANSIENT_BIT, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, etc.)

    if (vkCreateCommandPool(vkLogicDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool!");

#pragma endregion

#pragma region [Create command buffer]

#pragma endregion


    // Main loop
    while (true)
    {
        window.EventLoop();

        if (std::ranges::any_of(window.PopAllEvent(), [](const NWA::WindowEvent& event) -> bool { return event.type == NWA::WindowEvent::Type::Close; }))
            break;
    }


    // Clean up
    vkDestroyDevice(vkLogicDevice, nullptr);
    vkDestroyInstance(vkInstance, nullptr);

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