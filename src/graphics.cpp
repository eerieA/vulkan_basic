#include <precomp.h>
#include <graphics.h>
#include <GLFW/glfw3.h>

namespace veng {
    
    Graphics::Graphics(gsl::not_null<Window*> window) : window_(window) {
        InitializeVulkan();
    }
    Graphics::~Graphics(){
        if (instance_ != nullptr) {
            vkDestroyInstance(instance_, nullptr);
        }        
    }

    void Graphics::InitializeVulkan() {
        CreateInstance();
    }

    void Graphics::CreateInstance() {

        gsl::span<gsl::czstring> suggested_extensions = GetSuggestedExtensions();

        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pNext = nullptr;   // no extensions (custom)
        app_info.pApplicationName = "Vulkan simple";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "VEng";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instance_creation_info = {};
        instance_creation_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_creation_info.pNext = nullptr;
        instance_creation_info.pApplicationInfo = &app_info;
        instance_creation_info.enabledExtensionCount = suggested_extensions.size();
        instance_creation_info.ppEnabledExtensionNames = suggested_extensions.data();
        instance_creation_info.enabledLayerCount = 0;

        VkResult result = vkCreateInstance(&instance_creation_info, nullptr, &instance_);

        if (result != VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }
    }
    
    gsl::span<gsl::czstring> Graphics::GetSuggestedExtensions() {
        std::uint32_t glfw_extension_count = 0;
        gsl::czstring* glfw_extensions;
        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        return { glfw_extensions, glfw_extension_count};
    }
}