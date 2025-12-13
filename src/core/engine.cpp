#include "core/engine.h"

#include "core/application.h"

#include "imgui_impl_sdl3.h"
#include <SDL3/SDL.h>
#include <tracy/Tracy.hpp>

Engine::Engine(const char *name, uint32_t width, uint32_t height)
{
    ZoneScoped;

    application = eastl::make_unique<Application>(name, width, height);
    renderer = eastl::make_unique<Renderer>(application.get());
    physics = eastl::make_unique<Physics>();
    input = eastl::make_unique<Input>();
    camera = eastl::make_unique<Camera>(input.get());
    // world = eastl::make_unique<World>(physics.get(), input.get());
}

Engine::~Engine()
{
    ZoneScoped;
}

void Engine::run()
{
    ZoneScoped;

    auto previousTime = std::chrono::high_resolution_clock::now();

    running = true;
    while (running) {
        ZoneScopedN("Main loop");

        // calculate deltaTime
        auto  currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::milli>(previousTime - currentTime).count() / 1000;
        previousTime = currentTime;

        processInput(deltaTime);
        update(deltaTime);

        if (!minimized)
            renderer->draw();
    }
}

void Engine::processInput(float deltaTime)
{
    ZoneScoped;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        input->processEvent(event); // process event before everyting else
        // ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_WINDOW_MINIMIZED)
            minimized = true;
        else if (event.type == SDL_EVENT_WINDOW_RESTORED)
            minimized = false;

        if (event.type == SDL_EVENT_WINDOW_ENTER_FULLSCREEN)
            fullscreen = true;
        else if (event.type == SDL_EVENT_WINDOW_LEAVE_FULLSCREEN)
            fullscreen = false;

        // if (event.type == SDL_EVENT_WINDOW_RESIZED)
        //     renderer->requestResize();

        if (event.type == SDL_EVENT_QUIT)
            running = false;

        if (input->getKey(KeyboardKey::ESCAPE, InputAction::Pressed))
            running = false;

        // flyCamera.processInput(deltaTime);
        // world->processInput(deltaTime);
    }
}

void Engine::update(float deltaTime)
{
    ZoneScoped;

    // physics->update();
    // flyCamera.update(deltaTime);
    // world->update(deltaTime);
}