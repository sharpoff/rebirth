#pragma once

#include <rebirth/game/game.h>
#include <rebirth/graphics/renderer.h>
#include <rebirth/util/timer.h>

#include <SDL3/SDL.h>

class Application
{
public:
    Application(std::string name, unsigned int width, unsigned int height);
    ~Application();

    void run();

private:
    void handleInput(float deltaTime);
    void update(float deltaTime);
    void render();

    bool running = false;
    bool minimized = false;
    bool fullscreen = false;

    std::string name = "Application";
    uint32_t width = 0;
    uint32_t height = 0;

    util::Timer timer;
    SDL_Window *window;

    Renderer renderer;
    Camera camera;

    // hardcoded
    EntityID controlledCarEntityId = EntityID::Invalid;

    Scene scene;
};