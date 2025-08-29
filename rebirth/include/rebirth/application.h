#pragma once

#include <SDL3/SDL.h>
#include <string>

#include <rebirth/renderer.h>
#include <rebirth/util/timer.h>
#include <rebirth/types/scene.h>

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
    uint32_t width;
    uint32_t height;

    util::Timer timer;
    SDL_Window *window;

    Renderer *renderer;
    Camera camera;

    Scene scene;

    void handleInput(float deltaTime);
    void update(float deltaTime);
};

} // namespace rebirth