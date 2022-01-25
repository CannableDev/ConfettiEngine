#include "Application.h"
#include <map>
#include <set>

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
    else std::cout << "Debug Messenger Created\n";
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

void Application::createSurface()
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("ERROR: Failed to create window surface!");
    }
    else std::cout << "Display Surface Created\n"; 
}

void Application::pickPhysicalDevice()
{
    // find num of devices
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount <= 0) {
        throw std::runtime_error("ERROR: Failed to find GPUs with Vulkan support");
    }
    // get devices
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, &devices[0]);
    
    // map of devices
    std::multimap<int, VkPhysicalDevice> candidates;
    // check for best suitable device
    for (const auto& device : devices) {
        int score = getDeviceScore(device);
        candidates.insert(std::make_pair(score, device));
    }

    // saftey check
    if (candidates.rbegin()->first > 0) {
        physicalDevice = candidates.rbegin()->second;
        std::cout << "Physical Device Found\n";
    }
    else {
        throw std::runtime_error("ERROR: Failed to find GPUs with Vulkan support");
    }
}

int Application::getDeviceScore(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    
    QueueFamilyIndices indices = findQueueFamilies(device);
    
    int score = 0;

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    // Application can't function without geometry shaders or complete queue
    if (!deviceFeatures.geometryShader || !indices.isComplete()) {
        return 0;
    }

    return score;
}

QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, &queueFamilies[0]);

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        // checking for window presenting support in queue family
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) break;

        i++;
    }

    return indices;
}

void Application::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfoVec;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        // queue prio influences the scheduling of command buffer execution, is always required
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfoVec.push_back(queueCreateInfo);
    }

    // will use later, enables specific features on device
    VkPhysicalDeviceFeatures deviceFeatures{};

    // main create structure
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfoVec.size());
    createInfo.pQueueCreateInfos = &queueCreateInfoVec[0];
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;

    if (enableValidationLayers) {
        // ignored by up-to-date vulkan, but good backwards compatibility
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("ERROR: Failed to create logical device!");
    }
    else std::cout << "Logical Device Created\n";

    // setting graphics queues
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
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
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
}

void Application::mainLoop()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void Application::cleanup()
{
    // logical device doesn't directly interact with instance, so doesnt need to be destroyed
    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}
