#pragma once

#include "core/engine_stats.h"
#include "game/world.h"
#include "graphics/renderer.h"
#include "input/input.h"
#include "physics/physics.h"

#include "util/timer.h"

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
    uint32_t      width = 0;
    uint32_t      height = 0;

    Timer       timer;
    SDL_Window *window;
    EngineStats stats;

    Renderer renderer;
    Camera   camera;
    Physics  physics;
    Editor   editor;
    World    world;
    Input    input;
};