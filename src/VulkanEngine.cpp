#include <iostream>
#include "VulkanEngine.h"
#include <vulkan/vulkan.h>
#include "VkBootstrap.h"

void VulkanEngine::initVulkan() {
    vkb::InstanceBuilder instance_builder;
    auto instance_builder_return = instance_builder
            .set_app_name("vulkan-step-by-step")
            .request_validation_layers()
            .use_default_debug_messenger()
            .build ();;

    if (!instance_builder_return) {
        std::cerr << "Failed to create Vulkan instance. Error: " << instance_builder_return.error().message() << "\n";
        return;
    }
    vkb::Instance vkb_instance = instance_builder_return.value();
    _instance = vkb_instance.instance;
    VkResult error = glfwCreateWindowSurface (_instance, _window, NULL, &_surface);
    if (error != VK_SUCCESS) {
        std::cerr << "Failed to create glfw window surface. Error code: " << '\n' << error << '\n';
        return;
    }

    vkb::PhysicalDeviceSelector phys_device_selector (vkb_instance);
    auto physical_device_selector_return = phys_device_selector
            .set_surface(_surface)
            .select ();
    if (!physical_device_selector_return) {
        std::cerr << "Failed to set physical devices " << "\n";
    }
    _physicalDevices = physical_device_selector_return.value();
}

void VulkanEngine::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _window = glfwCreateWindow(800, 600, "Vulkan", nullptr, nullptr);
}

void VulkanEngine::init() {
    initWindow();
    initVulkan();
}
