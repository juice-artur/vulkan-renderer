#ifndef VULKAN_STEP_BY_STEP_VULKANENGINE_H
#define VULKAN_STEP_BY_STEP_VULKANENGINE_H
#include <vulkan/vulkan.h>
#include "vector"
#include "GLFW/glfw3.h"
#include "VkBootstrap.h"
#include "DeletionQueue.h"
#include "vk_types.h"

class VulkanEngine {
public:

    void init();
    void run();
    void cleanup();
private:
    VkExtent2D _windowExtent{ 800 , 600 };

    GLFWwindow* _window;
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;
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

    VkRenderPass _renderPass;
    std::vector<VkFramebuffer> _frameBuffers;

    VkSemaphore _presentSemaphore, _renderSemaphore;
    VkFence _renderFence;

    VkPipelineLayout _trianglePipelineLayout;
    VkPipeline _trianglePipeline;

    VmaAllocator _allocator;

    DeletionQueue _mainDeletionQueue;

    void initWindow();
    void initVulkan();
    void initSwapchain();
    void initCommands();
    void initDefaultRenderPass();
    void initFrameBuffers();
    void initSyncStructures();
    void initPipelines();

    void draw();
    bool loadShaderModule(const char* filePath, VkShaderModule* outShaderModule);
};


#endif
