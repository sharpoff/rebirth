#include <rebirth/core/application.h>
#include <rebirth/input/input.h>

#include <rebirth/graphics/gltf.h>

#include <rebirth/physics/physics_system.h>

#include <rebirth/util/filesystem.h>
#include <rebirth/util/logger.h>
#include <rebirth/core/cvar_system.h>

#include "backend/imgui_impl_sdl3.h"

#include <tracy/Tracy.hpp>

Application::Application(eastl::string name, unsigned int width, unsigned int height)
    : name(name), width(width), height(height)
{
    ZoneScopedN("Application init");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        logger::logError("Failed to initialize SDL", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window *window = SDL_CreateWindow(name.c_str(), width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        logger::logError("Failed to create SDL window", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    timer.start();
    renderer.initialize(window);

    // load scenes
    {
        ZoneScopedN("Load scenes");
        // if (!gltf::loadScene(renderer, scene, "assets/models/sponza/Sponza.gltf")) {
        // if (!gltf::loadScene(renderer, scene, "assets/models/subway_station/scene.gltf")) {
        if (!gltf::loadScene(renderer, scene, "assets/models/DamagedHelmet/DamagedHelmet.gltf")) {
            logger::logError("Failed to load scene.");
            exit(EXIT_FAILURE);
        }
    }

    // setup camera
    camera.setPerspectiveInf(glm::radians(60.0f), float(width) / height, 0.1f);
    camera.setPosition(vec3(0, 2, 2));
    camera.type = CameraType::FirstPerson;

    renderer.lights.push_back(
        Light{
            .type = LightType::Directional,
            .direction = vec3(0.0, -1.0, 0.0),
        });

    // physicsSystem.initialize();
    // Game::initialize();
}

Application::~Application()
{
    ZoneScopedN("Application shutdown");

    // Game::shutdown();
    // physicsSystem.shutdown();

    renderer.shutdown();

    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Application::run()
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

void Application::handleInput(float deltaTime)
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
        // enable imgui
        if (input.isKeyPressed(KeyboardKey::H)) {
            auto *cvar = CVarSystem::instance()->getCVarInt("render_imgui");
            if (CVarSystem::instance()->getCVarInt("render_imgui"))
                *cvar = !(*cvar);
        }

        // TODO: this is not working for some reason
        // enable fullscreen
        if (input.isKeyPressed(KeyboardKey::F)) {
            // fullscreen = !fullscreen;
            // SDL_SetWindowFullscreen(window, fullscreen);
            // renderer.requestResize();
        }

        if (input.isKeyPressed(KeyboardKey::Q)) {
            renderer.reloadShaders();
        }

        camera.handleEvent(event, deltaTime);

        // Game::processInput();
    }
}

void Application::update(float deltaTime)
{
    ZoneScopedN("Update");

    // Game::update(deltaTime);
    // physicsSystem.update(deltaTime);

    camera.update(deltaTime);
}

void Application::render()
{
    ZoneScopedN("Render");

    // Game::draw(renderer);

    renderer.drawScene(scene);
    renderer.present(camera);
}