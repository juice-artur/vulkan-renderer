#include "VulkanEngine.h"
int main(){
    VulkanEngine vulkanEngine{};
    vulkanEngine.init();
    glfwTerminate();
    return 0;
};