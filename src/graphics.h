#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include <glfw_window.h>

namespace veng {
    
class Graphics final {
    public:
    Graphics(gsl::not_null<Window*> window);
    ~Graphics();

    private:

    void InitializeVulkan();
    void CreateInstance();
    void SetupDebugMessenger();
    std::vector<gsl::czstring> GetRequiredInstanceExtensions();

    static gsl::span<gsl::czstring> GetSuggestedInstanceExtensions();
    static std::vector<VkExtensionProperties> GetSupportedInstanceExtensions();
    static bool AreAllExtensionSupported(gsl::span<gsl::czstring> extensions);

    static std::vector<VkLayerProperties> GetSupportedValidationLayers();
    static bool AreAllLayersSupported(gsl::span<gsl::czstring> extensions);

    VkInstance instance_ = nullptr;
    VkDebugUtilsMessengerEXT debug_messenger_;
    gsl::not_null<Window*> window_;
    bool validation_enabled_ = false;
};

}