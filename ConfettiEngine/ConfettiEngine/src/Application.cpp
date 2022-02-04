#include "Application.h"
#include <map>
#include <set>
#include <cstdint>
#include <algorithm>

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

    // Application can't function without geometry shaders or complete queue or required extensions
    if (!deviceFeatures.geometryShader || !indices.isComplete() || !checkDeviceExtensionSupport(device)) {
        return 0;
    }

    // Check for swap chain support, if no support in either format or present return 0
    SwapChainDetails chainSupport = querySwapChainSupport(device);
    if (chainSupport.formats.empty() || chainSupport.presentModes.empty()) {
        return 0;
    }

    return score;
}

bool Application::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, &availableExtensions[0]);

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
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

void Application::createSwapChain()
{
    SwapChainDetails swapChainSupport = querySwapChainSupport(physicalDevice);
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
    // recommended to have 1 more than min images in chain, but can't exceed max
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    // 0 is a special number that means there is no max
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    // image layers always 1 unless making steroscopic 3d apps
    createInfo.imageArrayLayers = 1;
    // specifies what kind of operations we'll use the images in the swap chain for
    // colour is direct display, post process would require it be VK_IMAGE_USAGE_TRANSFER_DST_BIT
    // post would also require memory operations to transfer final image eventually
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    // specifying handling swap chain over multiple queue families
    // if the families are different, we need concurrent image sharing. otherwise is not needed
    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }
    // pre transform if required (say rotating 90 degrees for mobile screen)
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    // specifies if alpha should be used for blending with other windows. Usually ignore
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // may be required to re-create chain from scratch say if window is resized
    // complex and will deal with later
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("ERROR: Failed to create swap chain!");
    }
    else std::cout << "Swap Chain Created\n";

    // must re-retrieve image count as we only specified minimum, so vulkan could create more
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
    // saving format and extents for later use
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

SwapChainDetails Application::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainDetails details;
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    // resize to fit all formats
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, &details.formats[0]);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    // resize to fit all present modes
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR Application::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == DesiredSurfaceFormat && availableFormat.colorSpace == DesiredColourSpace) {
            return availableFormat;
        }
    }
    // default to first available format if desired combo is not available
    return availableFormats[0];
}

VkPresentModeKHR Application::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == DesiredPresentationMode) {
            return availablePresentMode;
        }
    }
    // return default present mode if desired is not available
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Application::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    // if window manager gives UINT32_MAX we can pick our own resolution that best fits
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void Application::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        // how data should be interpreted
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        // color channel swizzling - this is default rgba
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        // subresource range = image's purpose and what part of image should be accessed
        // here we have no mipmapping or layers
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("ERROR: Failed to create image views!");
        }
        else std::cout << "Image View Created\n";
    }
}

void Application::createGraphicsPipeline()
{
    pipeline = new GraphicsPipeline(instance, device, swapChainExtent, swapChainImageFormat, "./src/Shaders/testTriangle.vert.spv","./src/Shaders/testTriangle.frag.spv");
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
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = &deviceExtensions[0];

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
    createSwapChain();
    createImageViews();
    createGraphicsPipeline();
}

void Application::mainLoop()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void Application::cleanup()
{
    delete(pipeline);

    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    
    vkDestroySwapchainKHR(device, swapChain, nullptr);
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
