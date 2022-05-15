#include "iostream"
#include "VulkanEngine.h"
int main(){
    std::cout <<"Hi";
    VulkanEngine vulkanEngine;
    vulkanEngine.initVulkan();
    glfwTerminate();
    return 0;
};