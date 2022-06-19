#include <iostream>
#include "VulkanEngine.h"
#include "VkBootstrap.h"
#include "vk_initializers.h"
#include "vk_pipeline.h"
#include <fstream>

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION

#include "vk_mem_alloc.h"
#include "vk_mem_alloc.h"
#include <gtx/transform.hpp>

#define VK_CHECK(x)                                                 \
    do                                                              \
    {                                                               \
        VkResult err = x;                                           \
        if (err)                                                    \
        {                                                           \
            std::cout <<"Detected Vulkan error: " << err << std::endl; \
            abort();                                                \
        }                                                           \
    } while (0)

//TODO: Add error information output
void VulkanEngine::init() {
    initWindow();
    initVulkan();
    initSwapchain();
    initCommands();
    initDefaultRenderPass();
    initFrameBuffers();
    initSyncStructures();
    initPipelines();
    loadMeshes();
    initScene();
}

void VulkanEngine::initVulkan() {
    vkb::InstanceBuilder instance_builder;
    auto instance_builder_return = instance_builder
            .set_app_name("vulkan-step-by-step")
            .request_validation_layers(true)
            .use_default_debug_messenger()
            .build();

    if (!instance_builder_return) {
        std::cerr << "Failed to create Vulkan instance. Error: " << instance_builder_return.error().message() << "\n";
        return;
    }

    vkb::Instance vkb_instance = instance_builder_return.value();
    _instance = vkb_instance.instance;
    _debugMessenger = vkb_instance.debug_messenger;

    VkResult error = glfwCreateWindowSurface(_instance, _window, nullptr, &_surface);
    if (error != VK_SUCCESS) {
        std::cerr << "Failed to create glfw window surface. Error code: " << '\n' << error << '\n';
        return;
    }

    vkb::PhysicalDeviceSelector selector{vkb_instance};
    vkb::PhysicalDevice physicalDevice = selector
            .set_minimum_version(1, 1)
            .set_surface(_surface)
            .select()
            .value();

    vkb::DeviceBuilder deviceBuilder{physicalDevice};

    vkb::Device vkbDevice = deviceBuilder.build().value();

    _device = vkbDevice.device;
    _chosenGPU = physicalDevice.physical_device;

    _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = _chosenGPU;
    allocatorInfo.device = _device;
    allocatorInfo.instance = _instance;
    vmaCreateAllocator(&allocatorInfo, &_allocator);
}

void VulkanEngine::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _window = glfwCreateWindow((int) _windowExtent.width, (int) _windowExtent.height, "Vulkan", nullptr, nullptr);
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void VulkanEngine::initSwapchain() {
    vkb::SwapchainBuilder swapchainBuilder{_chosenGPU, _device, _surface};

    vkb::Swapchain vkbSwapchain = swapchainBuilder
            .use_default_format_selection()
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(_windowExtent.width, _windowExtent.height)
            .build()
            .value();

    _swapchain = vkbSwapchain.swapchain;
    _swapchainImages = vkbSwapchain.get_images().value();
    _swapchainImageViews = vkbSwapchain.get_image_views().value();
    _swapchainImageFormat = vkbSwapchain.image_format;

    VkExtent3D depthImageExtent = {
            _windowExtent.width,
            _windowExtent.height,
            1
    };
    _depthFormat = VK_FORMAT_D32_SFLOAT;
    VkImageCreateInfo deepImgInfo = vkinit::imageCreateInfo(_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                            depthImageExtent);
    VmaAllocationCreateInfo deepImgAllocInfo = {};
    deepImgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    deepImgAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vmaCreateImage(_allocator, &deepImgInfo, &deepImgAllocInfo, &_depthImage._image, &_depthImage._allocation, nullptr);

    VkImageViewCreateInfo deepViewInfo = vkinit::imageviewCreateInfo(_depthFormat, _depthImage._image,
                                                                     VK_IMAGE_ASPECT_DEPTH_BIT);
    VK_CHECK(vkCreateImageView(_device, &deepViewInfo, nullptr, &_depthImageView));

    _mainDeletionQueue.push_function([=]() {
        vkDestroySwapchainKHR(_device, _swapchain, nullptr);
        vkDestroyImageView(_device, _depthImageView, nullptr);
        vmaDestroyImage(_allocator, _depthImage._image, _depthImage._allocation);
    });
}

void VulkanEngine::initCommands() {
    VkCommandPoolCreateInfo commandPoolInfo = vkinit::commandPoolCreateInfo(_graphicsQueueFamily,
                                                                            VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_commandPool);
    VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::commandBufferAllocateInfo(_commandPool, 1);
    VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_mainCommandBuffer));

    _mainDeletionQueue.push_function([=]() {
        vkDestroyCommandPool(_device, _commandPool, nullptr);
    });
}

void VulkanEngine::initDefaultRenderPass() {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = _swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment = {};
    depthAttachment.flags = 0;
    depthAttachment.format = _depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subPass = {};
    subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPass.colorAttachmentCount = 1;
    subPass.pColorAttachments = &colorAttachmentRef;
    subPass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDependency depthDependency = {};
    depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    depthDependency.dstSubpass = 0;
    depthDependency.srcStageMask =
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depthDependency.srcAccessMask = 0;
    depthDependency.dstStageMask =
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDependency dependencies[2] = {dependency, depthDependency};

    VkAttachmentDescription attachments[2] = {colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = &attachments[0];
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subPass;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = &dependencies[0];
    vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass);

    _mainDeletionQueue.push_function([=]() {
        vkDestroyRenderPass(_device, _renderPass, nullptr);
    });
}

void VulkanEngine::initFrameBuffers() {
    VkFramebufferCreateInfo fbInfo = {};
    fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.pNext = nullptr;

    fbInfo.renderPass = _renderPass;
    fbInfo.attachmentCount = 1;
    fbInfo.width = _windowExtent.width;
    fbInfo.height = _windowExtent.height;
    fbInfo.layers = 1;
    const uint32_t swapchainImageCount = _swapchainImages.size();
    _frameBuffers = std::vector<VkFramebuffer>(swapchainImageCount);

    for (int i = 0; i < swapchainImageCount; i++) {
        VkImageView attachments[2];
        attachments[0] = _swapchainImageViews[i];
        attachments[1] = _depthImageView;

        fbInfo.pAttachments = attachments;
        fbInfo.attachmentCount = 2;

        VK_CHECK(vkCreateFramebuffer(_device, &fbInfo, nullptr, &_frameBuffers[i]));

        _mainDeletionQueue.push_function([=]() {
            vkDestroyFramebuffer(_device, _frameBuffers[i], nullptr);
            vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
        });
    }
}

void VulkanEngine::initSyncStructures() {
    VkFenceCreateInfo fenceCreateInfo = vkinit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

    VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_renderFence));

    _mainDeletionQueue.push_function([=]() {
        vkDestroyFence(_device, _renderFence, nullptr);
    });

    VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphoreCreateInfo();
    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_presentSemaphore));
    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_renderSemaphore));

    _mainDeletionQueue.push_function([=]() {
        vkDestroySemaphore(_device, _presentSemaphore, nullptr);
        vkDestroySemaphore(_device, _renderSemaphore, nullptr);
    });
}

void VulkanEngine::draw() {
    VK_CHECK(vkWaitForFences(_device, 1, &_renderFence, true, 1000000000));
    VK_CHECK(vkResetFences(_device, 1, &_renderFence));
    uint32_t swapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, _presentSemaphore, nullptr, &swapchainImageIndex));
    VK_CHECK(vkResetCommandBuffer(_mainCommandBuffer, 0));

    VkCommandBuffer cmd = _mainCommandBuffer;
    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
    VkClearValue clearValue;
    clearValue.color = {{0, 0, 0.0f, 1.0f}};

    VkClearValue depthClear;
    depthClear.depthStencil.depth = 1.f;


    VkRenderPassBeginInfo rpInfo = {};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.pNext = nullptr;
    rpInfo.renderPass = _renderPass;
    rpInfo.renderArea.offset.x = 0;
    rpInfo.renderArea.offset.y = 0;
    rpInfo.renderArea.extent = _windowExtent;
    rpInfo.framebuffer = _frameBuffers[swapchainImageIndex];
    rpInfo.clearValueCount = 2;
    VkClearValue clearValues[] = {clearValue, depthClear};
    rpInfo.pClearValues = &clearValues[0];
    vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
    drawObjects(cmd, _renderables.data(), _renderables.size());

    vkCmdEndRenderPass(cmd);
    VK_CHECK(vkEndCommandBuffer(cmd));


    VkSubmitInfo submit = {};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.pNext = nullptr;
    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submit.pWaitDstStageMask = &waitStage;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &_presentSemaphore;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &_renderSemaphore;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
    VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, _renderFence));

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &_swapchain;
    presentInfo.swapchainCount = 1;
    presentInfo.pWaitSemaphores = &_renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &swapchainImageIndex;
    VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));
    _frameNumber++;
}

void VulkanEngine::run() {
    while (!glfwWindowShouldClose(_window)) {
        float currentFrame = glfwGetTime();
        _deltaTime = currentFrame - _lastFrame;
        _lastFrame = currentFrame;

        glfwPollEvents();
        processInput(_window);
        draw();
    }
}

void VulkanEngine::cleanup() {

    if (vkDeviceWaitIdle(_device)) {
        _mainDeletionQueue.flush();
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        vkDestroyDevice(_device, nullptr);
        vkDestroyInstance(_instance, nullptr);
        glfwTerminate();
    }
}

bool VulkanEngine::loadShaderModule(const char *filePath, VkShaderModule *outShaderModule) {
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        return false;
    }

    size_t fileSize = (size_t) file.tellg();

    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
    file.seekg(0);
    file.read((char *) buffer.data(), fileSize);
    file.close();

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    createInfo.codeSize = buffer.size() * sizeof(uint32_t);
    createInfo.pCode = buffer.data();
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return false;
    }
    *outShaderModule = shaderModule;
    return true;
}

void VulkanEngine::initPipelines() {

    VkPipelineLayoutCreateInfo meshPipelineLayoutInfo = vkinit::pipelineLayoutCreateInfo();

    VkPushConstantRange push_constant;
    push_constant.offset = 0;
    push_constant.size = sizeof(MeshPushConstants);
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    meshPipelineLayoutInfo.pPushConstantRanges = &push_constant;
    meshPipelineLayoutInfo.pushConstantRangeCount = 1;

    VkPipelineLayout meshPipelineLayout;
    VK_CHECK(vkCreatePipelineLayout(_device, &meshPipelineLayoutInfo, nullptr, &meshPipelineLayout));


    PipelineBuilder pipelineBuilder;

    pipelineBuilder.vertexInputInfo = vkinit::vertexInputStateCreateInfo();
    pipelineBuilder.inputAssembly = vkinit::inputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    pipelineBuilder.viewport.x = 0.0f;
    pipelineBuilder.viewport.y = 0.0f;
    pipelineBuilder.viewport.width = (float) _windowExtent.width;
    pipelineBuilder.viewport.height = (float) _windowExtent.height;
    pipelineBuilder.viewport.minDepth = 0.0f;
    pipelineBuilder.viewport.maxDepth = 1.0f;
    pipelineBuilder.scissor.offset = {0, 0};
    pipelineBuilder.scissor.extent = _windowExtent;

    pipelineBuilder.rasterizer = vkinit::rasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

    pipelineBuilder.multisampling = vkinit::multisamplingStateCreateInfo();

    pipelineBuilder.colorBlendAttachment = vkinit::colorBlendAttachmentState();

    pipelineBuilder.depthStencil = vkinit::depthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

    pipelineBuilder.pipelineLayout = meshPipelineLayout;

    VertexInputDescription vertexDescription = Vertex::getVertexDescription();

    //connect the pipeline builder vertex input info to the one we get from Vertex
    pipelineBuilder.vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
    pipelineBuilder.vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

    pipelineBuilder.vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
    pipelineBuilder.vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();


    VkShaderModule triangleFragShader;
    if (!loadShaderModule("../shaders/triangle.frag.spv", &triangleFragShader)) {
        std::cout << "Error when building the triangle fragment shader module" << std::endl;
    } else {
        std::cout << "Triangle fragment shader successfully loaded" << std::endl;
    }

    VkShaderModule meshVertShader;
    if (!loadShaderModule("../shaders/triangle.vert.spv", &meshVertShader)) {
        std::cout << "Error when building the triangle vertex shader module" << std::endl;

    } else {
        std::cout << "Triangle vertex shader successfully loaded" << std::endl;
    }

    //add the other shaders
    pipelineBuilder.shaderStages.push_back(
            vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, meshVertShader));

    //make sure that triangleFragShader is holding the compiled colored_triangle.frag
    pipelineBuilder.shaderStages.push_back(
            vkinit::pipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader));

    //build the mesh triangle pipeline
    VkPipeline meshPipeline = pipelineBuilder.buildPipeline(_device, _renderPass);

    createMaterial(meshPipeline, meshPipelineLayout, "defaultmesh");

    //deleting all of the vulkan shaders
    vkDestroyShaderModule(_device, meshVertShader, nullptr);
    vkDestroyShaderModule(_device, triangleFragShader, nullptr);



    //adding the pipelines to the deletion queue
    _mainDeletionQueue.push_function([=]() {
        vkDestroyPipeline(_device, meshPipeline, nullptr);
        vkDestroyPipelineLayout(_device, meshPipelineLayout, nullptr);
    });
}

void VulkanEngine::loadMeshes() {
    Mesh triangleMesh{};

    triangleMesh._vertices.resize(3);

    triangleMesh._vertices[0].position = {1.f, 1.f, 0.0f};
    triangleMesh._vertices[1].position = {-1.f, 1.f, 0.0f};
    triangleMesh._vertices[2].position = {0.f, -1.f, 0.0f};

    triangleMesh._vertices[0].color = {1.0f, 0.0f, 0.0f};
    triangleMesh._vertices[1].color = {0.0f, 1.0f, 0.0f};
    triangleMesh._vertices[2].color = {0.0f, 0.0f, 1.0f};
    uploadMesh(triangleMesh);

    Mesh bunnyMesh{};
    bunnyMesh.loadFromObj("../assets/bunny.obj");

    uploadMesh(bunnyMesh);

    _meshes["bunny"] = bunnyMesh;
    _meshes["triangle"] = triangleMesh;
}

void VulkanEngine::uploadMesh(Mesh &mesh) {
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = mesh._vertices.size() * sizeof(Vertex);
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VmaAllocationCreateInfo vmaallocInfo = {};
    vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo,
                             &mesh._vertexBuffer._buffer,
                             &mesh._vertexBuffer._allocation,
                             nullptr));

    _mainDeletionQueue.push_function([=]() {
        vmaDestroyBuffer(_allocator, mesh._vertexBuffer._buffer, mesh._vertexBuffer._allocation);
    });

    void *data;
    vmaMapMemory(_allocator, mesh._vertexBuffer._allocation, &data);
    memcpy(data, mesh._vertices.data(), mesh._vertices.size() * sizeof(Vertex));
    vmaUnmapMemory(_allocator, mesh._vertexBuffer._allocation);
}

Material *VulkanEngine::createMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string &name) {
    Material mat;
    mat.pipeline = pipeline;
    mat.pipelineLayout = layout;
    _materials[name] = mat;
    return &_materials[name];
}

Material *VulkanEngine::getMaterial(const std::string &name) {
    auto it = _materials.find(name);
    if (it == _materials.end()) {
        return nullptr;
    } else {
        return &(*it).second;
    }
}

Mesh *VulkanEngine::getMesh(const std::string &name) {
    auto it = _meshes.find(name);
    if (it == _meshes.end()) {
        return nullptr;
    } else {
        return &(*it).second;
    }
}

void VulkanEngine::initScene() {
    RenderObject monkey;
    monkey.mesh = getMesh("bunny");
    monkey.material = getMaterial("defaultmesh");
    monkey.transformMatrix = glm::mat4{1.0f};

    _renderables.push_back(monkey);

    for (int x = -20; x <= 20; x++) {
        for (int y = -20; y <= 20; y++) {

            RenderObject tri;
            tri.mesh = getMesh("triangle");
            tri.material = getMaterial("defaultmesh");
            glm::mat4 translation = glm::translate(glm::mat4{1.0}, glm::vec3(x, 0, y));
            glm::mat4 scale = glm::scale(glm::mat4{1.0}, glm::vec3(0.2, 0.2, 0.2));
            tri.transformMatrix = translation * scale;
            _renderables.push_back(tri);
        }
    }
}

void VulkanEngine::drawObjects(VkCommandBuffer cmd, RenderObject *first, int count) {

    glm::mat4 view = glm::lookAt(_cameraPos, _cameraPos + _cameraFront, _cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
    projection[1][1] *= -1;

    Mesh *lastMesh = nullptr;
    Material *lastMaterial = nullptr;
    for (int i = 0; i < count; i++) {
        RenderObject &object = first[i];
        if (object.material != lastMaterial) {

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, object.material->pipeline);
            lastMaterial = object.material;
        }


        glm::mat4 model = object.transformMatrix;
        glm::mat4 meshMatrix = projection * view * model;

        MeshPushConstants constants;
        constants.renderMatrix = meshMatrix;
        vkCmdPushConstants(cmd, object.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                           sizeof(MeshPushConstants), &constants);

        if (object.mesh != lastMesh) {
            VkDeviceSize offset = 0;
            vkCmdBindVertexBuffers(cmd, 0, 1, &object.mesh->_vertexBuffer._buffer, &offset);
            lastMesh = object.mesh;
        }
        vkCmdDraw(cmd, object.mesh->_vertices.size(), 1, 0, 0);
    }
}

void VulkanEngine::processInput(GLFWwindow *window) {
    mouseMovement(window);
    if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(_window, true);

    float cameraSpeed = 2.5 * _deltaTime;
    if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS)
        _cameraPos += cameraSpeed * _cameraFront;
    if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS)
        _cameraPos -= cameraSpeed * _cameraFront;
    if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS)
        _cameraPos -= glm::normalize(glm::cross(_cameraFront, _cameraUp)) * cameraSpeed;
    if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS)
        _cameraPos += glm::normalize(glm::cross(_cameraFront, _cameraUp)) * cameraSpeed;
}


void VulkanEngine::mouseMovement(GLFWwindow *window) {
    double xPos;
    double yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    glfwSetCursorPos(window, _windowExtent.width / 2.0, _windowExtent.height / 2.0);

    float xOffset = xPos - _lastX;
    float yOffset = _lastY - yPos;
    xOffset *= _sensitivity;
    yOffset *= _sensitivity;
    _yaw += xOffset;
    _pitch += yOffset;

    calculationDirection();
}

void VulkanEngine::calculationDirection() {
    glm::vec3 direction;
    direction.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    direction.y = sin(glm::radians(_pitch));
    direction.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
    _cameraFront = glm::normalize(direction);
}
