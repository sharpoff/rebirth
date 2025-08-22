#include <rebirth/application.h>
#include <rebirth/util/filesystem.h>
#include <rebirth/util/logger.h>

#include "backend/imgui_impl_sdl3.h"

namespace rebirth
{

Application::Application(std::string name, unsigned int width, unsigned int height) : width(width), height(height)
{
    // set consistent path
    util::setCurrentPath(util::getExecutablePath().parent_path().parent_path());

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        util::logError("Failed to initialize SDL", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window *window = SDL_CreateWindow(name.c_str(), width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        util::logError("Failed to create SDL window", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    renderer = new Renderer(window);

    // setup camera
    camera.setProjection(math::perspective(glm::radians(60.0f), float(width) / height, 0.1f, 300.0f));
    camera.setPosition(vec3(0, 0, 2));

    renderer->setCamera(&camera);
}

Application::~Application()
{
    delete renderer;

    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Application::run()
{
    running = true;
    while (running) {
        // get delta time
        static auto previousTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::milli>(currentTime - previousTime).count() / 1000;
        previousTime = currentTime;

        handleInput(deltaTime);
        update(deltaTime);
        renderer->render();
    }
}

void Application::handleInput(float deltaTime)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_WINDOW_RESIZED) {
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            if (w && h) {
                this->width = w;
                this->height = h;
            }

            renderer->requestResize();
        }

        if (event.type == SDL_EVENT_QUIT || (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
            running = false;
        }

        camera.handleEvent(event, deltaTime);
    }
}

void Application::update(float deltaTime)
{
    // update camera
    camera.update(deltaTime);
}

} // namespace rebirth