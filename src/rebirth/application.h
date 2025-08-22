#pragma once

#include <SDL3/SDL.h>
#include <rebirth/renderer.h>
#include <string>

namespace rebirth
{

class Application
{
public:
    Application(std::string name, unsigned int width, unsigned int height);
    ~Application();

    void run();

private:
    bool running = false;
    uint32_t width, height;

    SDL_Window *window;

    Renderer *renderer;
    Camera camera;

    void handleInput(float deltaTime);
    void update(float deltaTime);
};

} // namespace rebirth