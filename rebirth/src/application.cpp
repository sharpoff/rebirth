#include <rebirth/application.h>
#include <rebirth/input/input.h>

#include <rebirth/graphics/gltf.h>
#include <rebirth/graphics/render_settings.h>

#include <rebirth/physics/physics_system.h>

#include <rebirth/util/filesystem.h>
#include <rebirth/util/logger.h>

#include "backend/imgui_impl_sdl3.h"

#include <tracy/Tracy.hpp>

Application::Application(std::string name, unsigned int width, unsigned int height)
    : name(name), width(width), height(height)
{
    ZoneScopedN("Application init");

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

    timer.start();
    renderer.initialize(window);

    // load scenes
    {
        ZoneScopedN("Load scenes");
        // if (!gltf::loadScene(scene, "assets/models/subway_station/scene.gltf")) {
        if (!gltf::loadScene(scene, "assets/models/sponza/Sponza.gltf")) {
            util::logError("Failed to load scene.");
            exit(EXIT_FAILURE);
        }
    }

    // setup camera
    camera.setPerspective(glm::radians(60.0f), float(width) / height, 0.1f, 300.0f);
    camera.setPosition(vec3(0, 2, 2));
    camera.type = CameraType::FirstPerson;

    renderer.addLight(
        Light{
            .type = LightType::Directional,
            .direction = vec3(0.0, -1.0, 0.0),
        });

    // physics
    g_physicsSystem.initialize();

    Game::initialize();

    // create game entites
#if 0
    Transform transform(vec3(), glm::identity<quat>(), vec3(100.0f, 0.2f, 100.0f));
    Box *plane = new Box(renderer.getCubePrimitive(), transform, true);
    g_physicsSystem.setFriction(plane->getRigidBodyId(), 1.0f);
    Game::addEntity(plane);

    Car *car = new Car(vec3(0, 5, 0));
    controlledCarEntityId = Game::addEntity(car);
#endif
}

Application::~Application()
{
    ZoneScopedN("Application shutdown");

    Game::shutdown();

    g_physicsSystem.shutdown();
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
        ZoneScopedN("Main loop");
        float deltaTime = deltaTimer.elapsedMilliseconds() / 1000;
        deltaTimer.start();

        handleInput(deltaTime);
        update(deltaTime);
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

        if (event.type == SDL_EVENT_WINDOW_RESIZED ||
            event.type == SDL_EVENT_WINDOW_ENTER_FULLSCREEN ||
            event.type == SDL_EVENT_WINDOW_LEAVE_FULLSCREEN) {
            renderer.requestResize();
        }

        if (event.type == SDL_EVENT_QUIT || input.isKeyPressed(KeyboardKey::ESCAPE)) {
            running = false;
        }
        // enable imgui
        if (input.isKeyPressed(KeyboardKey::H)) {
            g_renderSettings.imgui = !g_renderSettings.imgui;
        }

        // enable fullscreen
        if (input.isKeyPressed(KeyboardKey::F)) {
            // TODO: this is not working for some reason
            // state.fullscreen = !state.fullscreen;
            // SDL_SetWindowFullscreen(window, state.fullscreen);
            // SDL_SyncWindow(window);
            // renderer.requestResize();
        }

#if 0
        if (input.isKeyPressed(KeyboardKey::Q)) {
            renderer.reloadShaders();
        }

        std::random_device random_device;
        std::mt19937 rng(random_device());
        std::uniform_real_distribution<float> real_dist(0.0f, 360.0f);
        std::uniform_int_distribution<int> int_dist(0, 1);

        // reset cars position
        if (input.isKeyPressed(KeyboardKey::R)) {
            RigidBodyID controlledCarRB = Game::getEntityById(controlledCarEntityId)->getRigidBodyId();
            RigidBody &controlledCar = g_physicsSystem.getRigidBody(controlledCarRB);

            controlledCar.setPosition(vec3(0, 3, 0));
            controlledCar.setRotation(glm::identity<quat>());
        }

        // create box
        if (input.isKeyPressed(KeyboardKey::F)) {
            Transform transform(vec3(0.0f, 10.0f, 0.0f), normalize(quat(glm::radians(real_dist(rng)), int_dist(rng), int_dist(rng), int_dist(rng))));

            Box *box = new Box(renderer.getCubePrimitive(), transform, false);
            g_physicsSystem.setLinearVelocity(box->getRigidBodyId(), vec3(0.0f, -5.0f, 0.0f));
            Game::addEntity(box);
        }

        // create plane
        if (input.isKeyPressed(KeyboardKey::T)) {
            Transform transform(vec3(0.0f, 10.0f, 0.0f), normalize(quat(glm::radians(real_dist(rng)), int_dist(rng), int_dist(rng), int_dist(rng))), vec3(3.0f, 0.2f, 0.2f));

            Box *box = new Box(renderer.getCubePrimitive(), transform, false);
            g_physicsSystem.setLinearVelocity(box->getRigidBodyId(), vec3(0.0f, -5.0f, 0.0f));
            Game::addEntity(box);
        }

        // create sphere
        if (input.isKeyPressed(KeyboardKey::V)) {
            Transform transform(vec3(0.0f, 10.0f, 0.0f));

            Sphere *sphere = new Sphere(renderer.getSpherePrimitive(), 1.0f, transform, false);
            g_physicsSystem.setLinearVelocity(sphere->getRigidBodyId(), vec3(0.0f, -5.0f, 0.0f));
            Game::addEntity(sphere);
        }
#endif

        camera.handleEvent(event, deltaTime);

        Game::processInput();
    }
}

void Application::update(float deltaTime)
{
    ZoneScopedN("Update");

    g_physicsSystem.update(deltaTime);

    Game::update(deltaTime);

    if (camera.type == CameraType::LookAt && controlledCarEntityId != EntityID::Invalid) {
        RigidBodyID controlledCarRB = Game::getEntityById(controlledCarEntityId)->getRigidBodyId();
        RigidBody &controlledCar = g_physicsSystem.getRigidBody(controlledCarRB);

        camera.setPosition(controlledCar.getPosition());
    }

    g_renderSettings.timestampDeltaMs = renderer.getTimestampDeltaMs();

    camera.update(deltaTime);
}

void Application::render()
{
    ZoneScopedN("Render");

    Game::draw(renderer);

    renderer.drawScene(scene, scene.transform);

    for (auto &light : g_resourceManager.lights) {
        Transform transform(light.position);
        transform.scale(vec3(0.5f, 0.5f, 0.5f));

        renderer.drawModel(renderer.getSpherePrimitive(), transform);
    }

    renderer.present(camera);
}