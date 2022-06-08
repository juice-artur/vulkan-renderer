#include <iostream>
#include "VulkanEngine.h"
#include "VkBootstrap.h"
#include "vk_initializers.h"

//TODO: Add error information output
void VulkanEngine::init() {
    initWindow();
    initVulkan();
    initSwapchain();
    initCommands();
    initDefaultRenderPass();
    initFrameBuffers();
}

void VulkanEngine::initVulkan() {
    vkb::InstanceBuilder instance_builder;
    auto instance_builder_return = instance_builder
            .set_app_name("vulkan-step-by-step")
            .request_validation_layers(true)
            .use_default_debug_messenger()
            .build ();

    if (!instance_builder_return) {
        std::cerr << "Failed to create Vulkan instance. Error: " << instance_builder_return.error().message() << "\n";
        return;
    }

    vkb::Instance vkb_instance = instance_builder_return.value();
    _instance = vkb_instance.instance;
    _debugMessenger = vkb_instance.debug_messenger;

    VkResult error = glfwCreateWindowSurface (_instance, _window, nullptr, &_surface);
    if (error != VK_SUCCESS) {
        std::cerr << "Failed to create glfw window surface. Error code: " << '\n' << error << '\n';
        return;
    }

    vkb::PhysicalDeviceSelector selector{ vkb_instance };
    vkb::PhysicalDevice physicalDevice = selector
            .set_minimum_version(1, 1)
            .set_surface(_surface)
            .select()
            .value();

    vkb::DeviceBuilder deviceBuilder{ physicalDevice };

    vkb::Device vkbDevice = deviceBuilder.build().value();

    _device = vkbDevice.device;
    _chosenGPU = physicalDevice.physical_device;

    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void VulkanEngine::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _window = glfwCreateWindow((int)_windowExtent.width, (int)_windowExtent.height, "Vulkan", nullptr, nullptr);
}

void VulkanEngine::initSwapchain() {
    vkb::SwapchainBuilder swapchainBuilder{_chosenGPU,_device,_surface };

    vkb::Swapchain vkbSwapchain = swapchainBuilder
            .use_default_format_selection()
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(_windowExtent.width, _windowExtent.height)
            .build()
            .value();

    _swapchain = vkbSwapchain.swapchain;
    _swapchainImages = vkbSwapchain.get_images().value();
    _swapchainImageViews = vkbSwapchain.get_image_views().value();
    _swapchainImageFormat = vkbSwapchain.image_format;
}

void VulkanEngine::initCommands() {
    VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_commandPool);
    VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_commandPool, 1);
    vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_mainCommandBuffer);
}

void VulkanEngine::initDefaultRenderPass() {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = _swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subPass = {};
    subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPass.colorAttachmentCount = 1;
    subPass.pColorAttachments = &color_attachment_ref;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &colorAttachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subPass;
    vkCreateRenderPass(_device, &render_pass_info, nullptr, &_renderPass);
}

void VulkanEngine::initFrameBuffers() {
    VkFramebufferCreateInfo fbInfo = {};
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.pNext = nullptr;

    fbInfo.renderPass = _renderPass;
    fbInfo.attachmentCount = 1;
    fbInfo.width = _windowExtent.width;
    fbInfo.height = _windowExtent.height;
    fbInfo.layers = 1;
    /*std::cout << _swapchainImages.size();*/
    const uint32_t swapchainImageCount = _swapchainImages.size();
    _frameBuffers = std::vector<VkFramebuffer>(swapchainImageCount);

    for (int i = 0; i < swapchainImageCount; i++) {

        fbInfo.pAttachments = &_swapchainImageViews[i];
        vkCreateFramebuffer(_device, &fbInfo, nullptr, &_frameBuffers[i]);
    }
}

void VulkanEngine::cleanup() {

    if (vkDeviceWaitIdle(_device)){
        vkDestroySwapchainKHR(_device, _swapchain, nullptr);
        vkDestroyRenderPass(_device, _renderPass, nullptr);
        for (int i = 0; i < _frameBuffers.size(); i++) {
            vkDestroyFramebuffer(_device, _frameBuffers[i], nullptr);
            vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
        }

        vkDestroyCommandPool(_device, _commandPool, nullptr);
        vkDestroySwapchainKHR(_device, _swapchain, nullptr);
        for (auto & _swapchainImageView : _swapchainImageViews) {

            vkDestroyImageView(_device, _swapchainImageView, nullptr);
        }

        vkDestroyDevice(_device, nullptr);
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkb::destroy_debug_utils_messenger(_instance, _debugMessenger);
        vkDestroyInstance(_instance, nullptr);
        glfwTerminate();
    }
}
