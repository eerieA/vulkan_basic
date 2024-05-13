#include <iostream>
#include <vulkan/vulkan.h>
#include <cstdint>
#include <cmath>

int main(std::int32_t argc, char** argv) {
    std::uint32_t count = 0;
    std::size_t size = 12;
    std::float_t s = 0.0f;

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.pApplicationName = "Test app";
    appInfo.pEngineName = "Test engine";

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;

    VkInstance instance;

    VkResult result = vkCreateInstance(&instanceInfo, 0, &instance);
    if (result == VK_SUCCESS) {
        std::cout << "Creation was successful." << std::endl;
    }
    
    return 0;
}