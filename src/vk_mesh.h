#ifndef VULKAN_STEP_BY_STEP_VK_MESH_H
#define VULKAN_STEP_BY_STEP_VK_MESH_H


#include "vk_types.h"
#include "vec3.hpp"
#include <vector>

struct Vertex {

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};

struct Mesh {
    std::vector<Vertex> _vertices;

    AllocatedBuffer _vertexBuffer;
};

#endif //VULKAN_STEP_BY_STEP_VK_MESH_H
