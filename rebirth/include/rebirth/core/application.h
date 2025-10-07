#pragma once

#include <rebirth/graphics/renderer.h>
#include <rebirth/util/timer.h>

#include <SDL3/SDL.h>

class Application
{
public:
    Application(eastl::string name, unsigned int width, unsigned int height);
    ~Application();

    void run();

private:
    void handleInput(float deltaTime);
    void update(float deltaTime);
    void render();

    bool running = false;
    bool minimized = false;
    bool fullscreen = false;

    eastl::string name = "Application";
    uint32_t width = 0;
    uint32_t height = 0;

    Timer timer;
    SDL_Window *window;

    Renderer renderer;
    Camera camera;

    Scene scene;
};