#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
#include "NativeWinApp/Window.h"
#include "NativeWinApp/Vulkan.h"
#include "vert.h"
#include "frag.h"

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

#pragma endregion

#pragma region [Logic device]

    VkDevice logicDevice;
    VkQueue deviceQueue;
    int queueFamilyIndex = -1;

    {
        uint32_t queueFamilyCount = 0;
        ::vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        ::vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

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

#pragma region [Render pass]

    VkRenderPass renderPass;

    {
        VkAttachmentDescription colorAttachment {};
        colorAttachment.format = swapchainFormat.format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        if (::vkCreateRenderPass(logicDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
            throw std::runtime_error("failed to create render pass!");
    }

#pragma endregion

#pragma region [Graphics pipeline]

    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    {
        auto CreateShaderModule = [&logicDevice](const unsigned char* code, size_t size) -> VkShaderModule
        {
            VkShaderModuleCreateInfo createInfo {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = size;
            createInfo.pCode = reinterpret_cast<const uint32_t*>(code);

            VkShaderModule shaderModule;
            if (::vkCreateShaderModule(logicDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
                throw std::runtime_error("failed to create shader module!");

            return shaderModule;
        };

        VkShaderModule vertShaderModule = CreateShaderModule(vert.data(), vert.size());
        VkShaderModule fragShaderModule = CreateShaderModule(frag.data(), frag.size());

        VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment {};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState {};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (::vkCreatePipelineLayout(logicDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout!");

        VkGraphicsPipelineCreateInfo pipelineInfo {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(logicDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
            throw std::runtime_error("failed to create graphics pipeline!");

        ::vkDestroyShaderModule(logicDevice, fragShaderModule, nullptr);
        ::vkDestroyShaderModule(logicDevice, vertShaderModule, nullptr);
    }

#pragma endregion

#pragma region [Frame buffer]

    std::vector<VkFramebuffer> swapchainFramebuffers;

    swapchainFramebuffers.resize(swapchainImageViews.size());

    for (size_t i = 0; i < swapchainImageViews.size(); i++)
    {
        VkImageView attachments[] = { swapchainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;

        if (::vkCreateFramebuffer(logicDevice, &framebufferInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to create framebuffer!");
    }

#pragma endregion

#pragma region [Command pool & buffer]

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    if (::vkCreateCommandPool(logicDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
        throw std::runtime_error("failed to create command pool!");

    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (::vkAllocateCommandBuffers(logicDevice, &allocInfo, &commandBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffers!");

#pragma endregion

#pragma region [Sync objects]

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    {
        VkFenceCreateInfo fenceInfo {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        VkSemaphoreCreateInfo semaphoreInfo {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (::vkCreateSemaphore(logicDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS)
            throw std::runtime_error("failed to create semaphore!");

        if (::vkCreateSemaphore(logicDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS)
            throw std::runtime_error("failed to create semaphore!");

        if (::vkCreateFence(logicDevice, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS)
            throw std::runtime_error("failed to create fence!");
    }


#pragma endregion

    // Main loop
    while (true)
    {
        window.EventLoop();

        if (std::ranges::any_of(window.PopAllEvent(), [](const NWA::WindowEvent& event) -> bool { return event.type == NWA::WindowEvent::Type::Close; }))
            break;

        ::vkWaitForFences(logicDevice, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
        ::vkResetFences(logicDevice, 1, &inFlightFence);

        uint32_t imageIndex;
        ::vkAcquireNextImageKHR(logicDevice, swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        ::vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);

        // record command buffer
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (::vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
                throw std::runtime_error("failed to begin recording command buffer!");

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = swapchainFramebuffers[imageIndex];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = swapchainExtent;

            VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            ::vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            ::vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapchainExtent.width);
            viewport.height = static_cast<float>(swapchainExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            ::vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = swapchainExtent;
            ::vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            ::vkCmdDraw(commandBuffer, 3, 1, 0, 0);

            ::vkCmdEndRenderPass(commandBuffer);

            if (::vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
                throw std::runtime_error("failed to record command buffer!");
        }

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (::vkQueueSubmit(deviceQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS)
            throw std::runtime_error("failed to submit draw command buffer!");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        ::vkQueuePresentKHR(deviceQueue, &presentInfo);
    }

    ::vkDeviceWaitIdle(logicDevice);

    // Clean up

    ::vkDestroySemaphore(logicDevice, imageAvailableSemaphore, nullptr);
    ::vkDestroySemaphore(logicDevice, renderFinishedSemaphore, nullptr);
    ::vkDestroyFence(logicDevice, inFlightFence, nullptr);

    ::vkDestroyCommandPool(logicDevice, commandPool, nullptr);

    for (auto framebuffer : swapchainFramebuffers)
        ::vkDestroyFramebuffer(logicDevice, framebuffer, nullptr);

    ::vkDestroyPipeline(logicDevice, graphicsPipeline, nullptr);
    ::vkDestroyPipelineLayout(logicDevice, pipelineLayout, nullptr);
    ::vkDestroyRenderPass(logicDevice, renderPass, nullptr);

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