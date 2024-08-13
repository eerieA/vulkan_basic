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

    struct QueueFamilyIndices {
        std::optional<std::uint32_t> graphics_family = std::nullopt;
        std::optional<std::uint32_t> presentation_family = std::nullopt;

        bool IsValid() const { return graphics_family.has_value() /*&& presentation_family.has_value()*/; }
    };
    
    void InitializeVulkan();
    void CreateInstance();
    void SetupDebugMessenger();
    void PickPhysicalDevice();
    std::vector<gsl::czstring> GetRequiredInstanceExtensions();

    static gsl::span<gsl::czstring> GetSuggestedInstanceExtensions();
    static std::vector<VkExtensionProperties> GetSupportedInstanceExtensions();
    static bool AreAllExtensionSupported(gsl::span<gsl::czstring> extensions);

    static std::vector<VkLayerProperties> GetSupportedValidationLayers();
    static bool AreAllLayersSupported(gsl::span<gsl::czstring> extensions);

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    bool IsDeviceSuitable(VkPhysicalDevice device);
    std::vector<VkPhysicalDevice> GetAvailableDevices();

    VkInstance instance_ = nullptr;
    VkPhysicalDevice physical_device = nullptr;
    VkDebugUtilsMessengerEXT debug_messenger_;
    gsl::not_null<Window*> window_;
    bool validation_enabled_ = false;
};

}