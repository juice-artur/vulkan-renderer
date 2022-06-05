#ifndef VULKAN_STEP_BY_STEP_VULKANENGINE_H
#define VULKAN_STEP_BY_STEP_VULKANENGINE_H
#include <vulkan/vulkan.h>
#include "vector"
#include "GLFW/glfw3.h"

class VulkanEngine {
public:
    //TODO: add cleanup
    void init();
private:
    GLFWwindow* _window;
    VkInstance _instance;
    VkPhysicalDevice _physicalDevices;
    VkSurfaceKHR _surface;

    void initWindow();
    void initVulkan();
};


#endif //VULKAN_STEP_BY_STEP_VULKANENGINE_H
