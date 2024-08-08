#include <precomp.h>
#include <graphics.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <spdlog/spdlog.h>

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

namespace veng {

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
    
    Graphics::Graphics(gsl::not_null<Window*> window) : window_(window) {

        #if !defined(NDEBUG)
        validation_enabled_ = true;
        #endif

        InitializeVulkan();
    }

    Graphics::~Graphics(){
        if (instance_ != nullptr) {

            // Destroy extensions before destroying the overall VkInstance
            if (debug_messenger_ != nullptr) {
                vkDestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
            }

            vkDestroyInstance(instance_, nullptr);
        }
    }

    void Graphics::InitializeVulkan() {
        CreateInstance();
        SetupDebugMessenger();
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
}