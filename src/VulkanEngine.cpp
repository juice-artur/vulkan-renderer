#include <iostream>
#include "VulkanEngine.h"
#include <vulkan/vulkan.h>
#include "VkBootstrap.h"
void VulkanEngine::init_Vulkan() {
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
}
