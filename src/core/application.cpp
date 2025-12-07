#include "core/application.h"

#include "core/logger.h"
#include <stdlib.h>

#include <SDL3/SDL.h>

Application::Application(const char *name, uint32_t width, uint32_t height)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LOGE("Failed to initialize SDL: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow(name, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        LOGE("Failed to create SDL window: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    this->name = name;
    this->width = width;
    this->height = height;
}

Application::~Application()
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

vec2 Application::getWindowSize()
{
    SDL_GetWindowSize(window, (int *)&width, (int *)&height);
    return vec2(width, height);
}