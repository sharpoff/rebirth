#pragma once

#include "game/world.h"
#include "graphics/renderer.h"
#include "physics/physics.h"
#include "core/scene.h"
#include "core/engine_stats.h"

#include <util/timer.h>

#include <SDL3/SDL.h>

class Engine
{
public:
    void initialize();
    void shutdown();

    void run();

protected:
    virtual void handleInput(float deltaTime);
    virtual void update(float deltaTime);
    virtual void render();

    bool running = false;
    bool minimized = false;
    bool fullscreen = false;

    eastl::string name = "Application";
    uint32_t width = 0;
    uint32_t height = 0;

    Timer timer;
    SDL_Window *window;
    EngineStats stats;

    World world;
    Renderer renderer;
    Physics physics;
    Camera camera;

    Scene scene;
};