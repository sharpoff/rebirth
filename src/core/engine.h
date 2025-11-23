#pragma once

#include "core/application.h"
#include "core/camera.h"
#include "core/engine_stats.h"
#include "editor/editor.h"
#include "game/world.h"
#include "input/input.h"
#include "physics/physics.h"
#include "render/renderer.h"

#include "EASTL/unique_ptr.h"

class Engine
{
public:
    Engine(const char *name, uint32_t width, uint32_t height);
    ~Engine();

    void run();

protected:
    void processInput(float deltaTime);
    void update(float deltaTime);
    void render();

    bool running = false;
    bool minimized = false;
    bool fullscreen = false;

    EngineStats stats;

    eastl::unique_ptr<Application> application;
    eastl::unique_ptr<Renderer>    renderer;
    eastl::unique_ptr<Physics>     physics;
    eastl::unique_ptr<Camera>      camera;
    eastl::unique_ptr<Input>       input;
    eastl::unique_ptr<Editor>      editor;
    eastl::unique_ptr<World>       world;
};