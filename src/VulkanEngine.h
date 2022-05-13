#ifndef VULKAN_STEP_BY_STEP_VULKANENGINE_H
#define VULKAN_STEP_BY_STEP_VULKANENGINE_H
#include <vulkan/vulkan.h>
#include "vector"

class VulkanEngine {

public:
    void init_Vulkan();
private:
    VkInstance _instance;
    std::vector<VkPhysicalDevice> _physicalDevices;
    VkPhysicalDeviceProperties _physicalDeviceProperties;
};


#endif //VULKAN_STEP_BY_STEP_VULKANENGINE_H
