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

        bool IsValid() const { return graphics_family.has_value() && presentation_family.has_value(); }
    };
    
    void InitializeVulkan();
    void CreateInstance();
    void SetupDebugMessenger();
    void PickPhysicalDevice();
    void CreateLogicalDeviceAndQueues();
    void CreateSurface();
    std::vector<gsl::czstring> GetRequiredInstanceExtensions();

    static gsl::span<gsl::czstring> GetSuggestedInstanceExtensions();
    static std::vector<VkExtensionProperties> GetSupportedInstanceExtensions();
    static bool AreAllExtensionSupported(gsl::span<gsl::czstring> extensions);

    static std::vector<VkLayerProperties> GetSupportedValidationLayers();
    static bool AreAllLayersSupported(gsl::span<gsl::czstring> extensions);

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    bool IsDeviceSuitable(VkPhysicalDevice device);
    std::vector<VkPhysicalDevice> GetAvailableDevices();

    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug_messenger_;

    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice logical_device_ = VK_NULL_HANDLE;
    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    VkQueue present_queue_ = VK_NULL_HANDLE;

    VkSurfaceKHR surface_ = VK_NULL_HANDLE;

    gsl::not_null<Window*> window_;
    bool validation_enabled_ = false;
};

}