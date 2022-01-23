#include "Application.h"

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

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    // using glfw to retrieve its required extensions
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    // will be updated later to include actual validation layers
    createInfo.enabledLayerCount = 0;

    // checking for optional extension support
    // getting number of extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    // filling extensions properties
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, &extensions[0]);

    std::cout << "Available VK Extensions:\n";
    for (const auto& extension : extensions)
        std::cout << " - " << extension.extensionName << '\n';

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
        throw std::runtime_error("failed to create instance!");
    }
    else {
        std::cout << "Vulkan Instance Successfully Created\n";
    }
}

void Application::initVulkan()
{
    createInstance();
}

void Application::mainLoop()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void Application::cleanup()
{
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}
