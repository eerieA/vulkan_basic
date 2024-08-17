#include <precomp.h>
#include <graphics.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <set>

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

    bool Graphics::IsDeviceSuitable(VkPhysicalDevice device) {
        QueueFamilyIndices families = FindQueueFamilies(device);
        return families.IsValid();
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
        device_info.enabledExtensionCount = 0;
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

    #pragma endregion

    Graphics::Graphics(gsl::not_null<Window*> window) : window_(window) {

        #if !defined(NDEBUG)
        validation_enabled_ = true;
        #endif

        InitializeVulkan();
    }

    Graphics::~Graphics(){
        if (logical_device_ != VK_NULL_HANDLE) {
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
    }

}