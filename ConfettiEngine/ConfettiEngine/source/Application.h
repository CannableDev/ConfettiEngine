#ifndef APPLICATION_H
#define APPLICATION_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

class Application {
public:
    void run();

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;
    const char* WIND_NAME = "Confetti Engine";

private:

    GLFWwindow* window;

    void initWindow();

    VkInstance instance;

    void createInstance();

    void initVulkan();

    void mainLoop();

    void cleanup();

};

#endif