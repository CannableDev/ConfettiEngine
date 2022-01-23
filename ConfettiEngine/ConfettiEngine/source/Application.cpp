#include "Application.h"

// extension function, proxy to load vkCreateDebugUtilsMessengerEXT function
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}
// extension function, proxy to destroy the debug messenger 
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void Application::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void Application::initWindow()
{
    glfwInit();

    // disabling opengl
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // disabling resizing for later
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, WIND_NAME, nullptr, nullptr);
}

void Application::createInstance() {
    // checking if we need validation layers
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("ERROR: Validation layers requested, but not available!");
    }

    // optional application information
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = WIND_NAME;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = WIND_NAME;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // mandatory instance creation information
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = getRequiredExtensions();

    createInfo.enabledExtensionCount = (uint32_t)extensions.size();
    createInfo.ppEnabledExtensionNames = &extensions[0];
    
    // debug create info to debug creating vulkan instance
    // this instance will automatically be used on vkCreateInstance and vkDestroyInstance
    // and will be destroyed along with the instance later
    VkDebugUtilsMessengerCreateInfoEXT  debugCreateInfo{};
    // if validation layers are needed, add them into create info
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        // debug
        setupDebugCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    // checking for optional extension support
    // getting number of extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    // filling extensions properties
    std::vector<VkExtensionProperties> extProp(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, &extProp[0]);

    std::cout << "Available VK Extensions:\n";
    for (const auto& ext : extProp)
        std::cout << " - " << ext.extensionName << '\n';

    /* Finding all glfw extensions
    bool foundAll = true;
    for (int i = 0; i < glfwExtensionCount; i++) {
        bool found = false;
        for (const auto& extension : extensions) {
            std::cout << "COMPARING " << glfwExtensions[i] << " AND " << extension.extensionName << std::endl;
            if (strcmp(glfwExtensions[i], extension.extensionName)) {
                found = true;
            }
        }
        if (!found) foundAll = false;
    }

    if (foundAll) std::cout << "ALL GLFW EXTENSIONS FOUND\n";
    else std::cout << "GLFW EXTENSIONS MISSING\n";*/

    // instance creation call
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("ERROR: Failed to create instance!");
    }
    else {
        std::cout << "Vulkan Instance Successfully Created\n";
    }
}

void Application::setupDebugMessenger()
{
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    setupDebugCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("ERROR: Failed to set up debug messenger!");
    }
}

void Application::setupDebugCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& ci)
{
    ci = {};
    ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    ci.pfnUserCallback = debugCallback;
    ci.pUserData = nullptr; // Optional
}

bool Application::checkValidationLayerSupport()
{
    // retrieve number of layers
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    // retrieve actual layers
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
    std::cout << "Validation layers found:\n";

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                std::cout << " - " << layerName << std::endl;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }
    std::cout << std::endl;
    return true;
}

std::vector<const char*> Application::getRequiredExtensions()
{
    // using glfw to retrieve its required extensions
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    // if validation layers are enabled, add debug extension
    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    // return
    return extensions;
}

void Application::initVulkan()
{
    createInstance();
    setupDebugMessenger();
}

void Application::mainLoop()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void Application::cleanup()
{
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}
