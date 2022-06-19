#ifndef VULKAN_STEP_BY_STEP_VULKANENGINE_H
#define VULKAN_STEP_BY_STEP_VULKANENGINE_H

#include <vulkan/vulkan.h>
#include <unordered_map>
#include "vector"
#include "GLFW/glfw3.h"
#include "VkBootstrap.h"
#include "DeletionQueue.h"
#include "vk_types.h"
#include "vk_mesh.h"

struct Material {
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
};

struct RenderObject {
    Mesh *mesh;

    Material *material;

    glm::mat4 transformMatrix;
};

class VulkanEngine {
public:

    void init();

    void run();

    void cleanup();

private:
    VkExtent2D _windowExtent{800, 600};

    GLFWwindow *_window;
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

    VmaAllocator _allocator;

    VkImageView _depthImageView;
    AllocatedImage _depthImage;
    VkFormat _depthFormat;

    std::vector<RenderObject> _renderables;
    std::unordered_map<std::string, Material> _materials;
    std::unordered_map<std::string, Mesh> _meshes;

    DeletionQueue _mainDeletionQueue;
    int _frameNumber = 0;

    glm::vec3 _cameraPos = glm::vec3(0.f, -6.f, -10.f);
    glm::vec3 _cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 _cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

    float _deltaTime = 0.0f;
    float _lastFrame = 0.0f;

    float _lastX = _windowExtent.width / 2.0;
    float _lastY = _windowExtent.height / 2.0;

    float _yaw = -90.0f;
    float _pitch = 0.0f;
    float _sensitivity = 0.05;

    void initWindow();

    void initVulkan();

    void initSwapchain();

    void initCommands();

    void initDefaultRenderPass();

    void initFrameBuffers();

    void initSyncStructures();

    void initPipelines();

    void draw();

    bool loadShaderModule(const char *filePath, VkShaderModule *outShaderModule);

    void loadMeshes();

    void uploadMesh(Mesh &mesh);

    void drawObjects(VkCommandBuffer cmd, RenderObject *first, int count);

    void initScene();

    Material *createMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string &name);

    Material *getMaterial(const std::string &name);

    Mesh *getMesh(const std::string &name);

    void processInput(GLFWwindow* window);

    void mouseMovement(GLFWwindow *window);

    void calculationDirection();
};


#endif
