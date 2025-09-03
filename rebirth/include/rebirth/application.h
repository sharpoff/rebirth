#pragma once

#include <rebirth/application_state.h>
#include <rebirth/game/game_state.h>
#include <rebirth/renderer.h>
#include <rebirth/util/timer.h>

#include <rebirth/physics/physics_car.h>
#include <rebirth/physics/physics_system.h>

#include <SDL3/SDL.h>

namespace rebirth
{

class Application
{
public:
    Application(std::string name, unsigned int width, unsigned int height);
    ~Application();

    void run();
    void handleInput(float deltaTime);
    void update(float deltaTime);

private:
    bool running = false;
    std::string name = "Application";
    uint32_t width = 0;
    uint32_t height = 0;
    ApplicationState state{};

    GameState gameState;

    util::Timer timer;
    SDL_Window *window;

    Renderer renderer;
    Camera camera;
    PhysicsCar car;

    Scene cube;
    Scene sphere;
    Scene cylinder;

    PhysicsSystem physicsSystem;
};

} // namespace rebirth