#ifndef VULKAN_STEP_BY_STEP_VK_INITIALIZERS_H
#define VULKAN_STEP_BY_STEP_VK_INITIALIZERS_H


#include <vulkan/vulkan_core.h>

namespace vkinit {

    VkCommandPoolCreateInfo command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

    VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

}

#endif //VULKAN_STEP_BY_STEP_VK_INITIALIZERS_H
