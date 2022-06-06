#include <iostream>
#include "VulkanEngine.h"
#include <vulkan/vulkan.h>
#include "VkBootstrap.h"

void VulkanEngine::init() {
    initWindow();
    initVulkan();
    initSwapchain();
}

void VulkanEngine::initVulkan() {
    vkb::InstanceBuilder instance_builder;
    auto instance_builder_return = instance_builder
            .set_app_name("vulkan-step-by-step")
            .request_validation_layers(true)
            .use_default_debug_messenger()
            .build ();;

    if (!instance_builder_return) {
        std::cerr << "Failed to create Vulkan instance. Error: " << instance_builder_return.error().message() << "\n";
        return;
    }

    vkb::Instance vkb_instance = instance_builder_return.value();
    _instance = vkb_instance.instance;
    _debug_messenger = vkb_instance.debug_messenger;

    VkResult error = glfwCreateWindowSurface (_instance, _window, NULL, &_surface);
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
}

void VulkanEngine::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _window = glfwCreateWindow(_windowExtent.width, _windowExtent.height, "Vulkan", nullptr, nullptr);
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

void VulkanEngine::cleanup() {
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);

    for (int i = 0; i < _swapchainImageViews.size(); i++) {

        vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
    }

    vkDestroyDevice(_device, nullptr);
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
    vkDestroyInstance(_instance, nullptr);
    glfwTerminate();
}
