#pragma once

#include "SDL3/SDL_video.h"
#include "glm/ext/vector_int2.hpp"

class Application
{
public:
    Application(const char *name, uint32_t width, uint32_t height);
    ~Application();

    SDL_Window *getHandle() { return window; }
    glm::ivec2 getWindowSize();

private:
    const char *name = "";
    uint32_t    width = 0;
    uint32_t    height = 0;

    SDL_Window *window = nullptr;
};