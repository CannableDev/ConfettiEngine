#include <iostream>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <sdl/SDL.h>

using namespace glm;

SDL_Window* window;
SDL_GLContext glContext;
SDL_Event event;

std::string mainWindowTitle;
std::string displayDetails;

bool quitApp = false;
bool fullScreen = true;
int windowWidth = 800;
int windowHeight = 600;
int screenWidth = 0;
int screenHeight = 0;

int main()
{
    std::cout << "Hello World!\n";
    vec3 potato(1, 2, 3);

    std::cout << glm::to_string(potato) << std::endl;

    /// SDL ///

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        std::cerr << "APP:ERROR: SDL2 video subsystem couldn't be initialized. Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (fullScreen == true) {
        window = SDL_CreateWindow(mainWindowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_VULKAN | SDL_WINDOW_FULLSCREEN);
    }
    else {
        window = SDL_CreateWindow(mainWindowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
    }

    return EXIT_SUCCESS;
}
