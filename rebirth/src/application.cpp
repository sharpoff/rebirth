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
    renderer = new Renderer(window);

    // load scenes
    std::vector<std::filesystem::path> texturePaths;
    if (!gltf::loadScene(&scene, *renderer, "assets/models/sponza/Sponza.gltf")) {
        util::logError("Failed to load scene.");
        exit(-1);
    }

    // setup camera
    camera.setPerspective(glm::radians(60.0f), float(width) / height, 0.1f, 300.0f);
    camera.setPosition(vec3(0, 0, 2));

    renderer->addLight(
        Light{
            .color = {0.0, 0.0, 0.0},
            .position = {-1.0, 30.0, 0.0},
            .type = LightType::Directional,
        }
    );

    renderer->setCamera(&camera);
}

Application::~Application()
{
    scene.destroy();
    delete renderer;

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

        renderer->drawScene(scene);
        renderer->present();
    }
}

void Application::handleInput(float deltaTime)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            renderer->requestResize();
        }

        if (event.type == SDL_EVENT_QUIT ||
            (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
            running = false;
        }

        camera.handleEvent(event, deltaTime);
    }
}

void Application::update(float deltaTime)
{
    // update first animation
    // scene.updateAnimation(deltaTime);

    // update camera
    camera.update(deltaTime);
}

} // namespace rebirth