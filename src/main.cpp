#include "VulkanEngine.h"
int main(){
    VulkanEngine vulkanEngine{};
    vulkanEngine.init();
    vulkanEngine.cleanup();
    return 0;
};