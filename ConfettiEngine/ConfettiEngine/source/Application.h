#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    // when checking for more than one valid value, once the struct is full we can return true
    bool isComplete() {
        return graphicsFamily.has_value();
    }
};

class Application {
public:
    void run();

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    const char* WIND_NAME = "Confetti Engine";

    // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT = diagnostic, 1
    // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT = informational, 16
    // VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT = warnings but not always errors, 256
    // VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT = errors or invalid, 4096
    static const int DEBUG_SEVERITY = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    // VKAPI_ATTR and VKAPI_CALL ensure the function has the right signature
    // for vulkan to call it
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData) {

        if (severity >= DEBUG_SEVERITY) {
            std::cerr << "Validation layer | " << getMessageTypeString(type) << callbackData->pMessage << std::endl;
        }

        return VK_FALSE;
    };

private:

    GLFWwindow* window;

    void initWindow();

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    bool checkValidationLayerSupport();

    std::vector<const char*> getRequiredExtensions();

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    void createInstance();   

    void setupDebugMessenger();
    void setupDebugCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& ci);

    // implicitly destroyed when instance is destroyed
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    void pickPhysicalDevice();
    int getDeviceScore(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    VkDevice device;
    // implicitly destroyed when instance is destroyed
    VkQueue graphicsQueue;
    void createLogicalDevice();

    void initVulkan();

    void mainLoop();

    void cleanup();

    static const char* getMessageTypeString(const int type) {
        const char* msg = "Unknown Message Type:\n";

        switch (type) {
        case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
            msg = "General Event:\n";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
            msg = "Possible Mistake/Specificaiton Violation:\n";
            break;
        case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
            msg = "Performance-Affecting Event:\n";
            break;
        }

        return msg;
    }
};

#endif