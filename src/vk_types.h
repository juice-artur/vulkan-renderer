#ifndef VULKAN_STEP_BY_STEP_VK_TYPES_H
#define VULKAN_STEP_BY_STEP_VK_TYPES_H

#include <vk_mem_alloc.h>
#include <glm.hpp>

struct AllocatedBuffer {
    VkBuffer _buffer;
    VmaAllocation _allocation;
};

struct AllocatedImage {
    VkImage _image;
    VmaAllocation _allocation;
};

struct MeshPushConstants {
    glm::vec4 data;
    glm::mat4 renderMatrix;
};

struct FrameData {
    VkSemaphore _presentSemaphore, _renderSemaphore;
    VkFence _renderFence;

    VkCommandPool _commandPool;
    VkCommandBuffer _mainCommandBuffer;
};

#endif //VULKAN_STEP_BY_STEP_VK_TYPES_H
