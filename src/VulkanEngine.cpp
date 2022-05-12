#include <iostream>
#include "VulkanEngine.h"
#include<vulkan/vulkan.h>

VkResult VulkanEngine::init_Vulkan() {
    VkResult result = VK_SUCCESS;
    VkApplicationInfo applicationInfo{
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            nullptr,
            "vulkan-step-by-step",
            VK_MAKE_VERSION(1, 0, 0),
            "vulkan-step-by-step",
            VK_MAKE_VERSION(1, 0, 0),
            VK_MAKE_VERSION(1, 3, 0)
    };

    VkInstanceCreateInfo instanceInfo{
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            nullptr,
            0,
            &applicationInfo,
            0,
            nullptr,
            0,
            nullptr
    };
    result = vkCreateInstance(&instanceInfo, nullptr, &_instance);

    if (result != VK_SUCCESS){
        std::cout << "Can not create instance";
        return result;
    }

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, nullptr);

    _physicalDevices.resize(physicalDeviceCount);
    vkEnumeratePhysicalDevices(_instance,
                               &physicalDeviceCount,
                               &_physicalDevices[0]);
    return result;
}
