#ifndef VULKAN_STEP_BY_STEP_VK_TYPES_H
#define VULKAN_STEP_BY_STEP_VK_TYPES_H

#include <vk_mem_alloc.h>

struct AllocatedBuffer {
    VkBuffer _buffer;
    VmaAllocation _allocation;
};

#endif //VULKAN_STEP_BY_STEP_VK_TYPES_H
