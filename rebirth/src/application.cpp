#include <rebirth/application.h>
#include <rebirth/gltf.h>
#include <rebirth/util/filesystem.h>
#include <rebirth/util/logger.h>

#include "backend/imgui_impl_sdl3.h"

namespace rebirth
{

Application::Application(std::string name, unsigned int width, unsigned int height)
    : width(width), height(height)
{
    // set consistent path
    util::setCurrentPath(util::getExecutablePath().parent_path().parent_path());

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        util::logError("Failed to initialize SDL", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window *window =
        SDL_CreateWindow(name.c_str(), width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        util::logError("Failed to create SDL window", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    timer.start();
    renderer.initialize(window);

    // load scenes
    // if (!gltf::loadScene(&scene, renderer, "assets/models/sponza/Sponza.gltf")) {
    //     util::logError("Failed to load scene.");
    //     exit(EXIT_FAILURE);
    // }

    Scene fox;
    if (!gltf::loadScene(&fox, renderer, "assets/models/Fox.gltf")) {
        util::logError("Failed to load scene.");
        exit(EXIT_FAILURE);
    }
    fox.transform.scale(vec3(0.05f));
    fox.transform.translate(vec3(-4, 0, 0));
    state.scenes.push_back(fox);

    Scene man;
    if (!gltf::loadScene(&man, renderer, "assets/models/CesiumMan.glb")) {
        util::logError("Failed to load scene.");
        exit(EXIT_FAILURE);
    }
    man.transform.scale(vec3(3.0f));
    state.scenes.push_back(man);

    // setup camera
    camera.setPerspective(glm::radians(60.0f), float(width) / height, 0.1f, 300.0f);
    camera.setPosition(vec3(0, 0, 2));

    renderer.addLight(
        Light{
            .position = {-1.0, 30.0, 0.0},
            .type = LightType::Directional,
        }
    );
}

Application::~Application()
{
    for (auto &scene : state.scenes)
        scene.destroy(renderer.getGraphics());

    renderer.shutdown();

    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Application::run()
{
    running = true;

    util::Timer deltaTimer;
    deltaTimer.start();

    while (running) {
        float deltaTime = deltaTimer.elapsedMilliseconds() / 1000;
        deltaTimer.start();

        handleInput(deltaTime);
        update(deltaTime);

        for (auto &scene : state.scenes) {
            renderer.drawScene(scene);
        }

        renderer.present(state, camera);
    }
}

void Application::handleInput(float deltaTime)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_WINDOW_RESIZED ||
            event.type == SDL_EVENT_WINDOW_ENTER_FULLSCREEN ||
            event.type == SDL_EVENT_WINDOW_LEAVE_FULLSCREEN) {
            renderer.requestResize();
        }

        if (event.type == SDL_EVENT_QUIT ||
            (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
            running = false;
        }

        // enable imgui
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_H) {
            state.imgui = !state.imgui;
        }

        // enable fullscreen
        if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_F) {
            // TODO: this is not working for some reason
            // state.fullscreen = !state.fullscreen;
            // SDL_SetWindowFullscreen(window, state.fullscreen);
            // SDL_SyncWindow(window);
            // renderer.requestResize();
        }

        camera.handleEvent(event, deltaTime);
    }
}

void Application::update(float deltaTime)
{
    for (auto &scene : state.scenes) {
        scene.updateAnimation(renderer.getGraphics(), deltaTime, scene.currentAnimation);
    }

    // update camera
    camera.update(deltaTime);
}

} // namespace rebirth