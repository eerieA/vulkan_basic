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

    struct SwapChainProperties {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;

        bool IsValid() const { return !formats.empty() && !present_modes.empty(); }
    };
    
    void InitializeVulkan();
    void CreateInstance();
    void SetupDebugMessenger();
    void PickPhysicalDevice();
    void CreateLogicalDeviceAndQueues();
    void CreateSurface();
    void CreateSwapChain();
    void CreateImageViews();
    void CreateRenderPass();
    void CreateGraphicsPipeline();

    std::vector<gsl::czstring> GetRequiredInstanceExtensions();

    static gsl::span<gsl::czstring> GetSuggestedInstanceExtensions();
    static std::vector<VkExtensionProperties> GetSupportedInstanceExtensions();
    static bool AreAllExtensionSupported(gsl::span<gsl::czstring> extensions);

    static std::vector<VkLayerProperties> GetSupportedValidationLayers();
    static bool AreAllLayersSupported(gsl::span<gsl::czstring> extensions);

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
    SwapChainProperties GetSwapChainProperties(VkPhysicalDevice device);
    bool IsDeviceSuitable(VkPhysicalDevice device);
    std::vector<VkPhysicalDevice> GetAvailableDevices();
    bool AreAllDeviceExtensionSupported(VkPhysicalDevice device);
    std::vector<VkExtensionProperties> GetDeviceAvailableExtensions(VkPhysicalDevice device);
    
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(gsl::span<VkSurfaceFormatKHR> formats);
    VkPresentModeKHR ChooseSwapPresentMode(gsl::span<VkPresentModeKHR> present_modes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    std::uint32_t ChooseSwapImageCount(const VkSurfaceCapabilitiesKHR& capabilities);

    VkShaderModule CreateShaderModule(gsl::span<std::uint8_t> buffer);

    std::array<gsl::czstring, 1> required_device_extensions_ = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkInstance instance_ = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug_messenger_;

    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    VkDevice logical_device_ = VK_NULL_HANDLE;
    VkQueue graphics_queue_ = VK_NULL_HANDLE;
    VkQueue present_queue_ = VK_NULL_HANDLE;

    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkSwapchainKHR swap_chain_ = VK_NULL_HANDLE;
    VkSurfaceFormatKHR surface_format_;
    VkPresentModeKHR present_mode_;
    VkExtent2D extent_;
    std::vector<VkImage> swap_chain_images_;
    std::vector<VkImageView> swap_chain_image_views_;

    VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
    VkRenderPass render_pass_ = VK_NULL_HANDLE;
    VkPipeline pipeline_ = VK_NULL_HANDLE;

    gsl::not_null<Window*> window_;
    bool validation_enabled_ = false;
};

}