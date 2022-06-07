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
    VkExtent2D _windowExtent{ 800 , 600 };

    GLFWwindow* _window;
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debug_messenger;
    VkPhysicalDevice _chosenGPU;
    VkDevice _device;
    VkSurfaceKHR _surface;

    VkSwapchainKHR _swapchain;
    VkFormat _swapchainImageFormat;
    std::vector<VkImage> _swapchainImages;
    std::vector<VkImageView> _swapchainImageViews;

    VkQueue _graphicsQueue;
    uint32_t _graphicsQueueFamily;
    VkCommandPool _commandPool;
    VkCommandBuffer _mainCommandBuffer;

    void initWindow();
    void initVulkan();
    void initSwapchain();
    void initCommands();
};


#endif
