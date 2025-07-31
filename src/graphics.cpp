#include <precomp.h>
#include <graphics.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <set>
#include <uniform_transformations.h>

#pragma region VK_FUNCTION_EXT_IMPL

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* info,
    const VkAllocationCallbacks* allocator,
    VkDebugUtilsMessengerEXT* debug_messenger
) {
    PFN_vkCreateDebugUtilsMessengerEXT function =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

    if (function != nullptr) {
        return function(instance, info, allocator, debug_messenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debug_messenger,
    const VkAllocationCallbacks* allocator
) {
    PFN_vkDestroyDebugUtilsMessengerEXT function =
        reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (function != nullptr) {
        function(instance, debug_messenger, allocator);
    }
}

#pragma endregion

namespace veng {

    #pragma region VALIDATION_LAYERS
    static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
        void* user_data
    ) {
        if(severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            spdlog::warn("Vulkan Validation: {}", callback_data->pMessage);
        } else {
            spdlog::error("Vulkan Error: {}", callback_data->pMessage);
        }
        return VK_FALSE;
    }

    static VkDebugUtilsMessengerCreateInfoEXT GetCreateMessengerInfo() {
        VkDebugUtilsMessengerCreateInfoEXT creation_info = {};
        creation_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        creation_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        
        creation_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

        creation_info.pfnUserCallback = ValidationCallback;
        creation_info.pUserData = nullptr;

        return creation_info;
    }    

    std::vector<VkLayerProperties> Graphics::GetSupportedValidationLayers() {
        std::uint32_t count;
        vkEnumerateInstanceLayerProperties(&count, nullptr);
        
        if (count == 0) {
            return {};
        }

        std::vector<VkLayerProperties> properties(count);
        vkEnumerateInstanceLayerProperties(&count, properties.data());
        return properties;
    }

    bool LayerMatchesName(gsl::czstring name, const VkLayerProperties& properties) {
        return veng::streq(properties.layerName, name);
    }

    bool IsLayerSupported(gsl::span<VkLayerProperties> layers, gsl::czstring name) {
        return std::any_of(
            layers.begin(), layers.end(), std::bind_front(LayerMatchesName, name));
    }

    bool Graphics::AreAllLayersSupported(gsl::span<gsl::czstring> layers) {

        std::vector<VkLayerProperties> supported_layers = GetSupportedValidationLayers();

        return std::all_of(
            layers.begin(), layers.end(),
            std::bind_front(IsLayerSupported, supported_layers));
    }

    void Graphics::SetupDebugMessenger() {
        if (!validation_enabled_) {
            return;
        }

        VkDebugUtilsMessengerCreateInfoEXT info = GetCreateMessengerInfo();
        VkResult result = vkCreateDebugUtilsMessengerEXT(instance_, &info, nullptr, &debug_messenger_);
        if (result != VK_SUCCESS) {
            spdlog::error("Cannot create debug messenger");
            return;
        }
    }
    
    #pragma endregion

    #pragma region INSTANCE_AND_EXTENSIONS
    gsl::span<gsl::czstring> Graphics::GetSuggestedInstanceExtensions() {
        std::uint32_t glfw_extension_count = 0;
        gsl::czstring* glfw_extensions;
        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        return { glfw_extensions, glfw_extension_count};
    }
    
    std::vector<gsl::czstring> Graphics::GetRequiredInstanceExtensions() {
        gsl::span<gsl::czstring> suggested_extensions = GetSuggestedInstanceExtensions();
        std::vector<gsl::czstring> required_extensions(suggested_extensions.size());
        std::copy(suggested_extensions.begin(), suggested_extensions.end(), required_extensions.begin());

        if (validation_enabled_) {
            required_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        if (!AreAllExtensionSupported(suggested_extensions)) {
            std::exit(EXIT_FAILURE);
        }

        return required_extensions;
    }

    std::vector<VkExtensionProperties> Graphics::GetSupportedInstanceExtensions() {
        std::uint32_t count;
        vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

        if (count == 0) {
            return {};
        }

        std::vector<VkExtensionProperties> properties(count);
        vkEnumerateInstanceExtensionProperties(nullptr, &count, properties.data());
        return properties;
    }
    
    bool ExtensionMatchesName(gsl::czstring name, const VkExtensionProperties& properties) {
        return veng::streq(properties.extensionName, name);
    }

    bool IsExtensionSupported(gsl::span<VkExtensionProperties> extensions, gsl::czstring name) {
        return std::any_of(
            extensions.begin(), extensions.end(), std::bind_front(ExtensionMatchesName, name));
    }

    bool Graphics::AreAllExtensionSupported(gsl::span<gsl::czstring> extensions) {
        std::vector<VkExtensionProperties> supported_extensions = GetSupportedInstanceExtensions();

        return std::all_of(
            extensions.begin(), extensions.end(),
            std::bind_front(IsExtensionSupported, supported_extensions));
    }
    
    void Graphics::CreateInstance() {
        std::array<gsl::czstring, 1> validation_layers = { "VK_LAYER_KHRONOS_validation" };
        if(!AreAllLayersSupported(validation_layers)) {
            validation_enabled_ = false;
        }

        std::vector<gsl::czstring> required_extensions = GetRequiredInstanceExtensions();

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
        instance_creation_info.enabledExtensionCount = required_extensions.size();
        instance_creation_info.ppEnabledExtensionNames = required_extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT messenger_creation_info = GetCreateMessengerInfo();
        if (validation_enabled_) {
            instance_creation_info.pNext = &messenger_creation_info;
            instance_creation_info.enabledLayerCount = validation_layers.size();
            instance_creation_info.ppEnabledLayerNames = validation_layers.data();
        } else {
            instance_creation_info.enabledLayerCount = 0;
            instance_creation_info.ppEnabledLayerNames = nullptr;
        }

        VkResult result = vkCreateInstance(&instance_creation_info, nullptr, &instance_);

        if (result != VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }
    }

    #pragma endregion

    #pragma region DEVICES_AND_QUEUES
    
    Graphics::QueueFamilyIndices Graphics::FindQueueFamilies(VkPhysicalDevice device) {
        std:uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, families.data());

        auto graphics_family_it =
            std::find_if(families.begin(), families.end(), [](const VkQueueFamilyProperties &props)
                         { return props.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT); });

        QueueFamilyIndices result;
        result.graphics_family = graphics_family_it - families.begin();

        for (std::uint32_t i = 0; i < families.size(); i++) {
            VkBool32 has_presentation_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &has_presentation_support);
            if (has_presentation_support) {
                result.presentation_family = i;
                break;
            }
        }

        return result;
    }
    
    Graphics::SwapChainProperties Graphics::GetSwapChainProperties(VkPhysicalDevice device) {
        SwapChainProperties properties;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &properties.capabilities);

        std::uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_count, nullptr);
        properties.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_count, properties.formats.data());

        std::uint32_t modes_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &modes_count, nullptr);
        properties.present_modes.resize(modes_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &modes_count, properties.present_modes.data());

        return properties;
    }

    std::vector<VkExtensionProperties> Graphics::GetDeviceAvailableExtensions(VkPhysicalDevice device) {
        std::uint32_t available_extensions_count;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &available_extensions_count, nullptr);
        std::vector<VkExtensionProperties> available_extensions(available_extensions_count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &available_extensions_count, available_extensions.data());
        
        return available_extensions;
    }

    bool Graphics::AreAllDeviceExtensionSupported(VkPhysicalDevice device) {
         std::vector<VkExtensionProperties> available_extensions = GetDeviceAvailableExtensions(device);

         return std::all_of(required_device_extensions_.begin(), required_device_extensions_.end(),
                            std::bind_front(IsExtensionSupported, available_extensions));
    }

    bool Graphics::IsDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices families = FindQueueFamilies(device);
        return families.IsValid() && AreAllDeviceExtensionSupported(device) && GetSwapChainProperties(device).IsValid();
    }

    void Graphics::PickPhysicalDevice() {
        std::vector<VkPhysicalDevice> devices = GetAvailableDevices();

        std::erase_if(devices, std::not_fn(std::bind_front(&Graphics::IsDeviceSuitable, this)));

        if (devices.empty()) {
            spdlog::error("No physical devices that match the criteria");
            std::exit(EXIT_FAILURE);
        }

        physical_device_ = devices[0];
    }

    std::vector<VkPhysicalDevice> Graphics::GetAvailableDevices() {
        std::uint32_t device_count;
        vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);

        if (device_count==0) {
            return {};
        }

        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());

        return devices;
    }
    
    void Graphics::CreateLogicalDeviceAndQueues() {
        QueueFamilyIndices picked_device_families = FindQueueFamilies(physical_device_);

        if (!picked_device_families.IsValid()) {
            std::exit(EXIT_FAILURE);
        }

        std::set<std::uint32_t> unique_queue_families = {
            picked_device_families.graphics_family.value(),
            picked_device_families.presentation_family.value()};
        
        std::float_t queue_priority = 1.0f;

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        for (std::uint32_t unique_queue_family : unique_queue_families) {
            VkDeviceQueueCreateInfo queue_info = {};
            queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info.queueFamilyIndex = unique_queue_family;
            queue_info.queueCount = 1;
            queue_info.pQueuePriorities = &queue_priority;
            queue_create_infos.push_back(queue_info);
        }

        VkPhysicalDeviceFeatures required_features = {};

        VkDeviceCreateInfo device_info = {};
        device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        device_info.queueCreateInfoCount = queue_create_infos.size();
        device_info.pQueueCreateInfos = queue_create_infos.data();
        device_info.pEnabledFeatures = &required_features;
        device_info.enabledExtensionCount = required_device_extensions_.size();
        device_info.ppEnabledExtensionNames = required_device_extensions_.data();
        device_info.enabledLayerCount = 0;  // deprecated for new Vulcan

        VkResult result = vkCreateDevice(physical_device_, &device_info, nullptr, &logical_device_);
        if (result != VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }

        vkGetDeviceQueue(logical_device_, picked_device_families.graphics_family.value(), 0, &graphics_queue_);
        vkGetDeviceQueue(logical_device_, picked_device_families.presentation_family.value(), 0, &present_queue_);
    }

    #pragma endregion

    #pragma region PRESENTATION

    void Graphics::CreateSurface() {
        VkResult result = glfwCreateWindowSurface(instance_, window_->GetHandle(), nullptr, &surface_);
        if (result != VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }
    }

    bool IsRgbaTypeFormat(const VkSurfaceFormatKHR& format_properties) {
        return format_properties.format == VK_FORMAT_R8G8B8A8_SRGB
        || format_properties.format == VK_FORMAT_B8G8R8A8_SRGB;
    }

    bool IsSrgbColorSpace(const VkSurfaceFormatKHR& format_properties) {
        return format_properties.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }

    bool IsCorrectFormat(const VkSurfaceFormatKHR& format_properties) {
        return IsRgbaTypeFormat(format_properties) && IsSrgbColorSpace(format_properties);
    }

    VkSurfaceFormatKHR Graphics::ChooseSwapSurfaceFormat(gsl::span<VkSurfaceFormatKHR> formats) {
        if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
            return {VkFormat::VK_FORMAT_R8G8B8A8_SRGB, VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        }

        auto it = std::find_if(formats.begin(), formats.end(), IsCorrectFormat);

        if (it != formats.end()) {
            return *it;
        }

        return formats[0];
    }

    bool IsMailboxPresentMode(const VkPresentModeKHR& mode) {
        return mode == VK_PRESENT_MODE_MAILBOX_KHR;
    }

    VkPresentModeKHR Graphics::ChooseSwapPresentMode(gsl::span<VkPresentModeKHR> present_modes) {
        if (std::any_of(present_modes.begin(), present_modes.end(), IsMailboxPresentMode)) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Graphics::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        constexpr std::uint32_t kInvalidSize = std::numeric_limits<std::uint32_t>::max();

        if (capabilities.currentExtent.width != kInvalidSize) {
            return capabilities.currentExtent;
        } else {
            glm::ivec2 size = window_->GetFramebufferSize();
            VkExtent2D actual_extent = {
                static_cast<std::uint32_t>(size.x),
                static_cast<std::uint32_t>(size.y),
            };

            actual_extent.width = std::clamp(
                actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actual_extent.height = std::clamp(
                actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actual_extent;
        }
    }

    std::uint32_t Graphics::ChooseSwapImageCount(const VkSurfaceCapabilitiesKHR& capabilities) {
        std::uint32_t image_count = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && capabilities.maxImageCount < image_count) {
            image_count = capabilities.maxImageCount;
        }

        return image_count;
    }

    void Graphics::CreateSwapChain() {
        SwapChainProperties properties = GetSwapChainProperties(physical_device_);

        surface_format_ = ChooseSwapSurfaceFormat(properties.formats);
        present_mode_ = ChooseSwapPresentMode(properties.present_modes);
        extent_ = ChooseSwapExtent(properties.capabilities);
        std::uint32_t image_count = ChooseSwapImageCount(properties.capabilities);

        VkSwapchainCreateInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = surface_;
        info.minImageCount = image_count;
        info.imageFormat = surface_format_.format;
        info.imageColorSpace = surface_format_.colorSpace;
        info.imageExtent = extent_;
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.presentMode = present_mode_;
        info.preTransform = properties.capabilities.currentTransform;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.clipped = VK_TRUE;
        info.oldSwapchain = VK_NULL_HANDLE;
        
        QueueFamilyIndices indices = FindQueueFamilies(physical_device_);

        if (indices.graphics_family != indices.presentation_family) {
            std::array<std::uint32_t, 2> family_indices = {
                indices.graphics_family.value(),
                indices.presentation_family.value(),
            };

            info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            info.queueFamilyIndexCount = family_indices.size();
            info.pQueueFamilyIndices = family_indices.data();
        } else {
            info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        VkResult result = vkCreateSwapchainKHR(logical_device_, &info, nullptr, &swap_chain_);
        if (result != VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }

        swap_chain_images_.resize(image_count);
        vkGetSwapchainImagesKHR(logical_device_, swap_chain_, &image_count, swap_chain_images_.data());
    }

    void Graphics::CreateImageViews() {
        swap_chain_image_views_.resize(swap_chain_images_.size());

        auto image_view_it = swap_chain_image_views_.begin();
        for (VkImage image : swap_chain_images_) {
            VkImageViewCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.image = image;
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = surface_format_.format;
            info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = 1;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount = 1;

            VkResult result = vkCreateImageView(logical_device_, &info, nullptr, &*image_view_it);
            if (result != VK_SUCCESS) {
                std::exit(EXIT_FAILURE);
            }
            image_view_it = std::next(image_view_it);
        }
    }

    #pragma endregion

    #pragma region GRAPHICS_PIPELINE

    VkShaderModule Graphics::CreateShaderModule(gsl::span<std::uint8_t> buffer) {
        if (buffer.empty()) {
            return VK_NULL_HANDLE;
        }

        VkShaderModuleCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.codeSize = buffer.size();
        info.pCode = reinterpret_cast<std::uint32_t*>(buffer.data());

        VkShaderModule shader_module;
        VkResult result = vkCreateShaderModule(logical_device_, &info, nullptr, &shader_module);
        if (result != VK_SUCCESS) {
            return VK_NULL_HANDLE;
        }

        return shader_module;
    }

    void Graphics::CreateGraphicsPipeline() {

        std::vector<std::uint8_t> basic_vertex_data = ReadFile("./basic.vert.spv");
        VkShaderModule vertex_shader = CreateShaderModule(basic_vertex_data);
        gsl::final_action destroy_vertex([this, vertex_shader]() {
            vkDestroyShaderModule(logical_device_, vertex_shader, nullptr);
        });

        std::vector<std::uint8_t> basic_fragment_data = ReadFile("./basic.frag.spv");
        VkShaderModule fragment_shader = CreateShaderModule(basic_fragment_data);
        gsl::final_action destroy_fragment([this, fragment_shader]() {
            vkDestroyShaderModule(logical_device_, fragment_shader, nullptr);
        });

        if (vertex_shader == VK_NULL_HANDLE || fragment_shader == VK_NULL_HANDLE) {
            std::exit(EXIT_FAILURE);
        }

        VkPipelineShaderStageCreateInfo vertex_stage_info = {};
        vertex_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertex_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertex_stage_info.module = vertex_shader;
        vertex_stage_info.pName = "main";

        VkPipelineShaderStageCreateInfo fragment_stage_info = {};
        fragment_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragment_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragment_stage_info.module = fragment_shader;
        fragment_stage_info.pName = "main";

        std::array<VkPipelineShaderStageCreateInfo, 2> stage_infos = {
            vertex_stage_info, fragment_stage_info
        };

        std::array<VkDynamicState, 2> dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
        dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_info.dynamicStateCount = dynamic_states.size();
        dynamic_state_info.pDynamicStates = dynamic_states.data();

        VkViewport viewport = GetViewport();
        VkRect2D scissor = GetScissor();

        VkPipelineViewportStateCreateInfo viewport_info = {};
        viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_info.viewportCount = 1;
        viewport_info.pViewports = &viewport;
        viewport_info.scissorCount = 1;
        viewport_info.pScissors = &scissor;

        auto vertex_binding_description = Vertex::GetBindingDescription();
        auto vertex_attribute_descriptions = Vertex::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = 1;
        vertex_input_info.pVertexBindingDescriptions = &vertex_binding_description;
        vertex_input_info.vertexAttributeDescriptionCount = vertex_attribute_descriptions.size();
        vertex_input_info.pVertexAttributeDescriptions = vertex_attribute_descriptions.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly_info = {};
        input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_info.primitiveRestartEnable = VK_FALSE;

        VkPipelineRasterizationStateCreateInfo rasterization_state_info = {};
        rasterization_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_info.depthClampEnable = VK_FALSE;
        rasterization_state_info.rasterizerDiscardEnable = VK_FALSE;
        rasterization_state_info.polygonMode = VK_POLYGON_MODE_FILL;
        rasterization_state_info.lineWidth = 1.0f;
        rasterization_state_info.cullMode = VK_CULL_MODE_NONE;
        rasterization_state_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterization_state_info.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling_info = {};
        multisampling_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling_info.sampleShadingEnable = VK_FALSE;
        multisampling_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState color_blend_attachment = {};
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        color_blend_attachment.blendEnable = VK_TRUE;
        color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo color_blending_info = {};
        color_blending_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blending_info.logicOpEnable = VK_FALSE;
        color_blending_info.attachmentCount = 1;
        color_blending_info.pAttachments = &color_blend_attachment;

        VkPipelineLayoutCreateInfo layout_info = {};
        layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        VkPushConstantRange model_matrix_range = {};
        model_matrix_range.offset = 0;
        model_matrix_range.size = sizeof(glm::mat4);
        model_matrix_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        layout_info.pushConstantRangeCount = 1;
        layout_info.pPushConstantRanges = &model_matrix_range;

        layout_info.setLayoutCount = 1;
        layout_info.pSetLayouts = &descriptor_set_layout_;

        VkResult layout_result =
            vkCreatePipelineLayout(logical_device_, &layout_info, nullptr, &pipeline_layout_);
        if (layout_result != VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }

        VkGraphicsPipelineCreateInfo pipeline_info = {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.stageCount = stage_infos.size();
        pipeline_info.pStages = stage_infos.data();
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &input_assembly_info;
        pipeline_info.pViewportState = &viewport_info;
        pipeline_info.pRasterizationState = &rasterization_state_info;
        pipeline_info.pMultisampleState = &multisampling_info;
        pipeline_info.pDepthStencilState = nullptr;
        pipeline_info.pColorBlendState = &color_blending_info;
        pipeline_info.pDynamicState = &dynamic_state_info;
        pipeline_info.layout = pipeline_layout_;
        pipeline_info.renderPass = render_pass_;
        pipeline_info.subpass = 0;

        VkResult pipeline_result = vkCreateGraphicsPipelines(
            logical_device_, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline_);
        if (pipeline_result != VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }
    }

    VkViewport Graphics::GetViewport() {
        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<std::float_t>(extent_.width);
        viewport.height = static_cast<std::float_t>(extent_.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        return viewport;
    }

    VkRect2D Graphics::GetScissor() {
        VkRect2D scissor = {};
        scissor.offset = { 0, 0 };
        scissor.extent = extent_;
        return scissor;
    }

    void Graphics::CreateRenderPass() {
        VkAttachmentDescription color_attachment = {};
        color_attachment.format = surface_format_.format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_ref = {};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription main_subpass = {};
        main_subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        main_subpass.colorAttachmentCount = 1;
        main_subpass.pColorAttachments = &color_attachment_ref;

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &color_attachment;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &main_subpass;

        VkResult result = vkCreateRenderPass(logical_device_, &render_pass_info, nullptr, &render_pass_);
        if (result != VK_SUCCESS) {
            std:exit(EXIT_FAILURE);
        }
    }

    #pragma endregion

    #pragma region DRAWING

    void Graphics::CreateFramebuffers() {
        swap_chain_framebuffers_.resize(swap_chain_image_views_.size());

        for (std::uint32_t i = 0; i < swap_chain_image_views_.size(); i++) {
            VkFramebufferCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            info.renderPass = render_pass_;
            info.attachmentCount = 1;
            info.pAttachments = &swap_chain_image_views_[i];
            info.width = extent_.width;
            info.height = extent_.height;
            info.layers = 1;

            VkResult result =
                vkCreateFramebuffer(logical_device_, &info, nullptr, &swap_chain_framebuffers_[i]);
            if (result != VK_SUCCESS) {
                std::exit(EXIT_FAILURE);
            }
        }
    }

    void Graphics::CreateCommandPool() {
        QueueFamilyIndices indices = FindQueueFamilies(physical_device_);
        VkCommandPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_info.queueFamilyIndex = indices.graphics_family.value();

        VkResult result = vkCreateCommandPool(logical_device_, &pool_info, nullptr, &command_pool_);
        if (result != VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }
    }

    void Graphics::CreateCommandBuffer() {
        VkCommandBufferAllocateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.commandPool = command_pool_;
        info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        info.commandBufferCount = 1;

        VkResult result = vkAllocateCommandBuffers(logical_device_, &info, &command_buffer_);
        if (result != VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }
    }

    void Graphics::BeginCommands() {
        vkResetCommandBuffer(command_buffer_, 0);
        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VkResult begin_state = vkBeginCommandBuffer(command_buffer_, &begin_info);
        if (begin_state != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin command buffer!");
        }

        VkRenderPassBeginInfo render_pass_begin_info = {};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = render_pass_;
        render_pass_begin_info.framebuffer = swap_chain_framebuffers_[current_image_index_];
        render_pass_begin_info.renderArea.offset = { 0,0 };
        render_pass_begin_info.renderArea.extent = extent_;

        VkClearValue clear_color = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
        render_pass_begin_info.clearValueCount = 1;
        render_pass_begin_info.pClearValues = &clear_color;
        vkCmdBeginRenderPass(command_buffer_, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
        VkViewport viewport = GetViewport();
        VkRect2D scissor = GetScissor();

        vkCmdSetViewport(command_buffer_, 0, 1, &viewport);
        vkCmdSetScissor(command_buffer_, 0, 1, &scissor);
    }

    void Graphics::EndCommands() {
        vkCmdEndRenderPass(command_buffer_);
        VkResult end_buffer_result = vkEndCommandBuffer(command_buffer_);
        if (end_buffer_result != VK_SUCCESS) {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }

    void Graphics::CreateSignals() {
        VkSemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(logical_device_, &semaphore_info, nullptr, &image_available_signal_) !=
            VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }

        if (vkCreateSemaphore(logical_device_, &semaphore_info, nullptr, &render_finished_signal_) !=
            VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }

        VkFenceCreateInfo fence_info = {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateFence(logical_device_, &fence_info, nullptr, &still_rendering_fence_) !=
            VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }
    }

    void Graphics::CreateDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uniform_layout_binding = {};
        uniform_layout_binding.binding = 0;
        uniform_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniform_layout_binding.descriptorCount = 1;
        uniform_layout_binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;

        VkDescriptorSetLayoutCreateInfo layout_info = {};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = 1;
        layout_info.pBindings = &uniform_layout_binding;

        VkResult result = vkCreateDescriptorSetLayout(logical_device_, &layout_info, nullptr, &descriptor_set_layout_);
        if (result != VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }
    }

    bool Graphics::BeginFrame() {
        vkWaitForFences(logical_device_, 1, &still_rendering_fence_, VK_TRUE, UINT64_MAX);

        VkResult image_acquire_result = vkAcquireNextImageKHR(
            logical_device_,
            swap_chain_,
            UINT64_MAX,
            image_available_signal_,
            VK_NULL_HANDLE,
            &current_image_index_);

        if (image_acquire_result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateSwapChain();
            return false;
        }

        if (image_acquire_result != VK_SUCCESS && image_acquire_result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Couldn't acquire render image!");
        }

        vkResetFences(logical_device_, 1, &still_rendering_fence_);
        BeginCommands();
        SetModelMatrix(glm::mat4(1.0f));
        return true;
    }

    void Graphics::EndFrame() {
        EndCommands();

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &image_available_signal_;
        submit_info.pWaitDstStageMask = &wait_stage;

        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer_;

        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &render_finished_signal_;

        VkResult submit_result = vkQueueSubmit(graphics_queue_, 1, &submit_info, still_rendering_fence_);
        if (submit_result != VK_SUCCESS) {
            throw std::runtime_error("Failed to submit draw commands!");
        }

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &render_finished_signal_;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swap_chain_;
        present_info.pImageIndices = &current_image_index_;

        VkResult result = vkQueuePresentKHR(present_queue_, &present_info);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to present swap chain image!");
        }
    }

    void Graphics::RecreateSwapChain() {
        glm::ivec2 size = window_->GetFramebufferSize();
        if (size.x == 0 || size.y == 0) {
            size = window_->GetFramebufferSize();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(logical_device_);
        CleanupSwapChain();

        CreateSwapChain();
        CreateImageViews();
        CreateFramebuffers();
    }

    void Graphics::CleanupSwapChain() {
        if (logical_device_ == VK_NULL_HANDLE) {
            return;
        }

        for (VkFramebuffer framebuffer : swap_chain_framebuffers_) {
            vkDestroyFramebuffer(logical_device_, framebuffer, nullptr);
        }

        for (VkImageView image_view : swap_chain_image_views_) {
            vkDestroyImageView(logical_device_, image_view, nullptr);
        }

        if (swap_chain_ != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(logical_device_, swap_chain_, nullptr);
        }
    }

    #pragma endregion

    #pragma region BUFFERS

    std::uint32_t Graphics::FindMemoryType(std::uint32_t type_bits_filter, VkMemoryPropertyFlags required_properties) {
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(physical_device_, &memory_properties);
        gsl::span<VkMemoryType> memory_types(memory_properties.memoryTypes, memory_properties.memoryTypeCount);

        for (std::uint32_t i = 0; i < memory_types.size(); i++) {
            bool passes_filter = type_bits_filter & (1 << i);
            bool has_property_flags = memory_types[i].propertyFlags & required_properties;

            if (passes_filter && has_property_flags) {
                return i;
            }
        }

        throw std::runtime_error("Cannot find memory type!");
    }

    BufferHandle Graphics::CreateBuffer(
        VkDeviceSize size, VkBufferCreateFlags usage, VkMemoryPropertyFlags properties) {

        BufferHandle handle = {};

        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = usage;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkResult result = vkCreateBuffer(logical_device_, &buffer_info, nullptr, &handle.buffer);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create vertex buffer!");
        }

        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(logical_device_, handle.buffer, &memory_requirements);

        std::uint32_t chosen_memory_type = FindMemoryType(memory_requirements.memoryTypeBits, properties);

        VkMemoryAllocateInfo allocation_info = {};
        allocation_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocation_info.allocationSize = memory_requirements.size;
        allocation_info.memoryTypeIndex = chosen_memory_type;

        VkResult allocation_result =
            vkAllocateMemory(logical_device_, &allocation_info, nullptr, &handle.memory);

        if (allocation_result != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate buffer memory!");
        }

        vkBindBufferMemory(logical_device_, handle.buffer, handle.memory, 0);

        return handle;
    }

    BufferHandle Graphics::CreateIndexBuffer(gsl::span<std::uint32_t> indices) {
        VkDeviceSize size = sizeof(std::uint32_t) * indices.size();

        BufferHandle staging_handle = CreateBuffer(
            size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* data;
        vkMapMemory(logical_device_, staging_handle.memory, 0, size, 0, &data);
        std::memcpy(data, indices.data(), size);
        vkUnmapMemory(logical_device_, staging_handle.memory);

        BufferHandle gpu_handle = CreateBuffer(
            size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkCommandBuffer transient_commands = BeginTransientCommandBuffer();

        VkBufferCopy copy_info = {};
        copy_info.srcOffset = 0;
        copy_info.dstOffset = 0;
        copy_info.size = size;
        vkCmdCopyBuffer(transient_commands, staging_handle.buffer, gpu_handle.buffer, 1, &copy_info);

        EndTransientCommandBuffer(transient_commands);

        DestroyBuffer(staging_handle);

        return gpu_handle;
    }

    BufferHandle Graphics::CreateVertexBuffer(gsl::span<Vertex> vertices) {
        VkDeviceSize size = sizeof(Vertex) * vertices.size();
        BufferHandle staging_handle = CreateBuffer(
            size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        void* data;
        vkMapMemory(logical_device_, staging_handle.memory, 0, size, 0, &data);
        std::memcpy(data, vertices.data(), size);
        vkUnmapMemory(logical_device_, staging_handle.memory);

        BufferHandle gpu_handle = CreateBuffer(
            size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        VkCommandBuffer transient_commands = BeginTransientCommandBuffer();

        VkBufferCopy copy_info = {};
        copy_info.srcOffset = 0;
        copy_info.dstOffset = 0;
        copy_info.size = size;
        vkCmdCopyBuffer(transient_commands, staging_handle.buffer, gpu_handle.buffer, 1, &copy_info);

        EndTransientCommandBuffer(transient_commands);

        DestroyBuffer(staging_handle);
        
        return gpu_handle;
    }

    void Graphics::DestroyBuffer(BufferHandle handle) {
        vkDeviceWaitIdle(logical_device_);
        vkDestroyBuffer(logical_device_, handle.buffer, nullptr);
        vkFreeMemory(logical_device_, handle.memory, nullptr);
    }

    void Graphics::RenderBuffer(BufferHandle handle, std::uint32_t vertex_count) {
        VkDeviceSize offset = 0;
        vkCmdBindDescriptorSets(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1, &descriptor_set_, 0, nullptr);
        vkCmdBindVertexBuffers(command_buffer_, 0, 1, &handle.buffer, &offset);
        vkCmdDraw(command_buffer_, vertex_count, 1, 0, 0);
    }

    void Graphics::RenderIndexedBuffer(
        BufferHandle vertex_buffer, BufferHandle index_buffer, std::uint32_t count) {
        VkDeviceSize offset = 0;
        vkCmdBindDescriptorSets(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout_, 0, 1, &descriptor_set_, 0, nullptr);
        vkCmdBindVertexBuffers(command_buffer_, 0, 1, &vertex_buffer.buffer, &offset);
        vkCmdBindIndexBuffer(command_buffer_, index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(command_buffer_, count, 1, 0, 0, 0);
        SetModelMatrix(glm::mat4(1.0f));    // Reset model matrix
    }

    void Graphics::SetModelMatrix(glm::mat4 model) {
        vkCmdPushConstants(
            command_buffer_, pipeline_layout_, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model);
    }

    void Graphics::SetViewProjection(glm::mat4 view, glm::mat4 projection) {
        UniformTransformations transformations(view, projection);
        std::memcpy(uniform_buffer_location_, &transformations, sizeof(UniformTransformations));
    }

    VkCommandBuffer Graphics::BeginTransientCommandBuffer() {
        VkCommandBufferAllocateInfo allocation_info = {};
        allocation_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocation_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocation_info.commandPool = command_pool_;
        allocation_info.commandBufferCount = 1;

        VkCommandBuffer buffer;
        vkAllocateCommandBuffers(logical_device_, &allocation_info, &buffer);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(buffer, &begin_info);

        return buffer;
    }

    void Graphics::EndTransientCommandBuffer(VkCommandBuffer command_buffer) {
        vkEndCommandBuffer(command_buffer);

        VkSubmitInfo  submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;

        vkQueueSubmit(graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphics_queue_);
        vkFreeCommandBuffers(logical_device_, command_pool_, 1, &command_buffer);
    }

    void Graphics::CreateUniformBuffers() {
        VkDeviceSize buffer_size = sizeof(UniformTransformations);
        uniform_buffer_ = CreateBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkMapMemory(
            logical_device_, uniform_buffer_.memory, 0, buffer_size, 0, &uniform_buffer_location_);
    }

    void Graphics::CreateDescriptorPool() {
        VkDescriptorPoolSize pool_size = {};
        pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        pool_size.descriptorCount = 1;

        VkDescriptorPoolCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        create_info.poolSizeCount = 1;
        create_info.pPoolSizes = &pool_size;
        create_info.maxSets = 1;

        VkResult result = vkCreateDescriptorPool(logical_device_, &create_info, nullptr, &descriptor_pool_);
        if (result != VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }
    }

    void Graphics::CreateDescriptorSet() {
        VkDescriptorSetAllocateInfo set_info = {};
        set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        set_info.descriptorPool = descriptor_pool_;
        set_info.descriptorSetCount = 1;
        set_info.pSetLayouts = &descriptor_set_layout_;

        VkResult result = vkAllocateDescriptorSets(logical_device_, &set_info, &descriptor_set_);
        if (result != VK_SUCCESS) {
            std::exit(EXIT_FAILURE);
        }

        VkDescriptorBufferInfo buffer_info = {};
        buffer_info.buffer = uniform_buffer_.buffer;
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UniformTransformations);

        VkWriteDescriptorSet descriptor_write = {};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = descriptor_set_;
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &buffer_info;

        vkUpdateDescriptorSets(logical_device_, 1, &descriptor_write, 0, nullptr);
    }

    #pragma endregion

    #pragma region CLASS

    Graphics::Graphics(gsl::not_null<Window*> window) : window_(window) {

        #if !defined(NDEBUG)
        validation_enabled_ = true;
        #endif

        InitializeVulkan();
    }

    Graphics::~Graphics(){

        if (logical_device_ != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(logical_device_);

            CleanupSwapChain();

            if (descriptor_pool_ != VK_NULL_HANDLE) {
                vkDestroyDescriptorPool(logical_device_, descriptor_pool_, nullptr);
            }

            DestroyBuffer(uniform_buffer_);

            if (descriptor_set_layout_ != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(logical_device_, descriptor_set_layout_, nullptr);
            }

            if (image_available_signal_ != VK_NULL_HANDLE) {
                vkDestroySemaphore(logical_device_, image_available_signal_, nullptr);
            }

            if (render_finished_signal_ != VK_NULL_HANDLE) {
                vkDestroySemaphore(logical_device_, render_finished_signal_, nullptr);
            }

            if (still_rendering_fence_ != VK_NULL_HANDLE) {
                vkDestroyFence(logical_device_, still_rendering_fence_, nullptr);
            }

            if (command_pool_ != VK_NULL_HANDLE) {
                vkDestroyCommandPool(logical_device_, command_pool_, nullptr);
            }

            if (pipeline_ != VK_NULL_HANDLE) {
                vkDestroyPipeline(logical_device_, pipeline_, nullptr);
            }

            if (pipeline_layout_ != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(logical_device_, pipeline_layout_, nullptr);
            }

            if (render_pass_ != VK_NULL_HANDLE) {
                vkDestroyRenderPass(logical_device_, render_pass_, nullptr);
            }

            vkDestroyDevice(logical_device_, nullptr);
        }

        if (instance_ != VK_NULL_HANDLE) {
            if (surface_ != VK_NULL_HANDLE) {
                vkDestroySurfaceKHR(instance_, surface_, nullptr);
            }

            // Destroy extensions before destroying the overall VkInstance
            if (debug_messenger_ != VK_NULL_HANDLE) {
                vkDestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
            }

            vkDestroyInstance(instance_, nullptr);
        }
    }

    void Graphics::InitializeVulkan() {
        CreateInstance();
        SetupDebugMessenger();
        CreateSurface();
        PickPhysicalDevice();
        CreateLogicalDeviceAndQueues();
        CreateSwapChain();
        CreateImageViews();
        CreateRenderPass();
        CreateDescriptorSetLayout();
        CreateGraphicsPipeline();
        CreateFramebuffers();
        CreateCommandPool();
        CreateCommandBuffer();
        CreateSignals();
        CreateUniformBuffers();
        CreateDescriptorPool();
        CreateDescriptorSet();
    }

    #pragma endregion
}