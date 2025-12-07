#pragma once

#ifndef NDEBUG
#define ENABLE_ENGINE_DEBUG
#endif

#include "core/application.h"
#include "core/camera.h"
#include "world/world.h"
#include "input/input.h"
#include "physics/physics.h"
#include "render/renderer.h"

#include "core/stl.h"

class Engine
{
public:
    Engine(const char *name, uint32_t width, uint32_t height);
    ~Engine();

    void run();

protected:
    void processInput(float deltaTime);
    void update(float deltaTime);

    bool running = false;
    bool minimized = false;
    bool fullscreen = false;

    UniquePtr<Application> application;
    UniquePtr<Renderer>    renderer;
    UniquePtr<Physics>     physics;
    UniquePtr<Camera>      camera;
    UniquePtr<Input>       input;
    UniquePtr<World>       world;
};