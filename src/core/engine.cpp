#include "core/engine.h"

#include "core/globals.h"
#include "input/input.h"
#include "graphics/gltf.h"
#include "util/logger.h"
#include "core/resource_manager.h"

#include "backend/imgui_impl_sdl3.h"
#include <tracy/Tracy.hpp>

void Engine::initialize()
{
    ZoneScopedN("Application init");

    width = 1280;
    height = 720;
    timer.start();

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        logger::logError("Failed to initialize SDL", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window *window = SDL_CreateWindow("Application", width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        logger::logError("Failed to create SDL window", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    renderer.initialize(window, &stats);
    physics.initialize();

    // setup camera
    camera.setPerspectiveInf(glm::radians(60.0f), float(width) / height, 0.1f);
    camera.setPosition(vec3(0, 2, 2));
    camera.type = CameraType::FirstPerson;

    ResourceManager::get()->addLight(
        GPULight{
            .type = LightType::Directional,
            .direction = vec3(0.0, -1.0, 0.0),
        });

    logger::logInfo("Engine initialized");

    // load scenes
    // if (!gltf::loadScene(renderer.getGraphics(), scene, "assets/models/sponza/Sponza.gltf")) {
    if (!gltf::loadScene(renderer.getGraphics(), scene, "assets/models/DamagedHelmet.glb")) {
        logger::logError("Failed to load scene.");
        exit(EXIT_FAILURE);
    }

    world.initialize(physics);
}

void Engine::shutdown()
{
    ZoneScopedN("Application shutdown");

    world.shutdown();
    physics.shutdown();
    renderer.shutdown();

    SDL_DestroyWindow(window);
    SDL_Quit();

    logger::logInfo("Engine shutdown");
}

void Engine::run()
{
    running = true;

    Timer deltaTimer;
    deltaTimer.start();

    while (running) {
        ZoneScopedN("Main loop");
        float deltaTime = deltaTimer.elapsedMilliseconds() / 1000;
        deltaTimer.start();

        handleInput(deltaTime);
        update(deltaTime);

        if (!minimized)
            render();
    }
}

void Engine::handleInput(float deltaTime)
{
    ZoneScopedN("Handle input");

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        Input &input = g_input;
        input.processEvent(&event);

        if (event.type == SDL_EVENT_WINDOW_MINIMIZED)
            minimized = true;
        else if (event.type == SDL_EVENT_WINDOW_RESTORED)
            minimized = false;

        if (event.type == SDL_EVENT_WINDOW_ENTER_FULLSCREEN)
            fullscreen = true;
        else if (event.type == SDL_EVENT_WINDOW_LEAVE_FULLSCREEN)
            fullscreen = false;

        if (event.type == SDL_EVENT_WINDOW_RESIZED)
            renderer.requestResize();

        if (event.type == SDL_EVENT_QUIT || input.isKeyPressed(KeyboardKey::ESCAPE)) {
            running = false;
        }

        if (input.isKeyPressed(KeyboardKey::E)) {
            Globals::isEditorOpened = !Globals::isEditorOpened;
        }

        if (input.isKeyPressed(KeyboardKey::R)) {
            renderer.reloadShaders();
        }

        camera.handleEvent(event, deltaTime);
    }
}

void Engine::update(float deltaTime)
{
    ZoneScopedN("Update");

    world.update(deltaTime, physics);
    physics.update(deltaTime);
    camera.update(deltaTime);
}

void Engine::render()
{
    ZoneScopedN("Render");

    for (auto &entity : world.getEntities()) {
        renderer.drawEntity(entity, DrawMask::Opaque | DrawMask::Shadow);
    }

    renderer.present(camera);
}