#ifndef VULKAN_STEP_BY_STEP_VULKANENGINE_H
#define VULKAN_STEP_BY_STEP_VULKANENGINE_H
#include <vulkan/vulkan.h>
#include "vector"
#include "GLFW/glfw3.h"
#include "VkBootstrap.h"

class VulkanEngine {
public:

    void init();
    void cleanup();
private:
    GLFWwindow* _window;
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debug_messenger;
    VkPhysicalDevice _chosenGPU;
    VkDevice _device;
    VkSurfaceKHR _surface;

    void initWindow();
    void initVulkan();
};


#endif //VULKAN_STEP_BY_STEP_VULKANENGINE_H
