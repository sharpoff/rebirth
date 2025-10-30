#pragma once

#include "core/application_info.h"
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
    void initialize(const ApplicationInfo &appInfo);
    void shutdown();

    void run();

protected:
    virtual void processInput(float deltaTime);
    virtual void update(float deltaTime);
    virtual void render();

    bool running = false;
    bool minimized = false;
    bool fullscreen = false;

    ApplicationInfo appInfo{};

    Timer       timer;
    SDL_Window *window;
    EngineStats stats;

    Renderer renderer;
    Camera   flyCamera;
    Physics  physics;
    Editor   editor;
    World    world;
    Input    input;
};