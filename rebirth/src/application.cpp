#include <rebirth/application.h>
#include <rebirth/input/input.h>

#include <rebirth/gltf.h>
#include <rebirth/util/filesystem.h>
#include <rebirth/util/logger.h>

#include "backend/imgui_impl_sdl3.h"

#include <random>

namespace rebirth
{

Application::Application(std::string name, unsigned int width, unsigned int height)
    : name(name), width(width), height(height)
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

    // Scene fox;
    // if (!gltf::loadScene(&fox, renderer, "assets/models/Fox.gltf")) {
    //     util::logError("Failed to load scene.");
    //     exit(EXIT_FAILURE);
    // }
    // fox.transform.scale(vec3(0.05f));
    // fox.transform.translate(vec3(-4, 0, 0));
    // state.scenes.push_back(fox);

    // Scene man;
    // if (!gltf::loadScene(&man, renderer, "assets/models/CesiumMan.glb")) {
    //     util::logError("Failed to load scene.");
    //     exit(EXIT_FAILURE);
    // }
    // man.transform.scale(vec3(3.0f));
    // state.scenes.push_back(man);

    if (!gltf::loadScene(cube, renderer, "assets/models/cube.glb")) {
        util::logError("Failed to load scene.");
        exit(EXIT_FAILURE);
    }

    if (!gltf::loadScene(sphere, renderer, "assets/models/sphere.glb")) {
        util::logError("Failed to load scene.");
        exit(EXIT_FAILURE);
    }

    if (!gltf::loadScene(cylinder, renderer, "assets/models/cylinder.glb")) {
        util::logError("Failed to load scene.");
        exit(EXIT_FAILURE);
    }

    // setup camera
    camera.setPerspective(glm::radians(60.0f), float(width) / height, 0.1f, 300.0f);
    camera.setPosition(vec3(0, 2, 2));
    camera.type = CameraType::LookAt;
    // camera.type = CameraType::FirstPerson;

    renderer.addLight(
        Light{
            .position = {-1.0, 30.0, 0.0},
            .type = LightType::Directional,
        }
    );

    // physics
    physicsSystem.initialize();

    // create game objects
    // plane
    Object planeObj(cube, cube.transform, "Plane");
    planeObj.transform.setScale(vec3(30.0f, 0.2f, 30.0f));
    planeObj.transform.translate(vec3(0, -5, 0));
    planeObj.aabb = calculateAABB(planeObj.scene, planeObj.transform * cube.transform);

    planeObj.rigidBodyId =
        physicsSystem.createBox(planeObj.transform, planeObj.aabb.getHalfExtent(), true);
    physicsSystem.setFriction(planeObj.rigidBodyId, 1.0f);

    gameState.objects.push_back(planeObj);

    // car
    car.initialize(physicsSystem, vec3(0, 3, 0));
}

Application::~Application()
{
    Graphics &graphics = renderer.getGraphics();
    for (auto &object : gameState.objects) {
        object.scene.destroy(graphics);
    }

    renderer.shutdown();
    physicsSystem.shutdown();

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

        for (auto &object : gameState.objects) {
            renderer.drawObject(object);
        }

        // draw wheels
        for (int i = 0; i < 4; i++) {
            renderer.drawScene(cylinder, Transform(car.getWheelTransform(i)));
            Transform transform(car.getPosition(), car.getRotation(), vec3(1.0f, 0.02f, 1.6f));
            renderer.drawScene(cube, transform);
        }

        renderer.present(state, camera);
    }
}

void Application::handleInput(float deltaTime)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        Input &input = Input::getInstance();
        input.processEvent(&event);

        if (event.type == SDL_EVENT_WINDOW_RESIZED ||
            event.type == SDL_EVENT_WINDOW_ENTER_FULLSCREEN ||
            event.type == SDL_EVENT_WINDOW_LEAVE_FULLSCREEN) {
            renderer.requestResize();
        }

        if (event.type == SDL_EVENT_QUIT || input.isKeyPressed(KeyboardKey::ESCAPE)) {
            running = false;
        }

        std::random_device random_device;
        std::mt19937 rng(random_device());
        std::uniform_real_distribution<float> real_dist(0.0f, 360.0f);
        std::uniform_int_distribution<int> int_dist(0, 1);

        // create box
        if (input.isKeyPressed(KeyboardKey::R)) {
            car.setPosition(vec3(0, 3, 0));
            car.setRotation(glm::identity<quat>());
        }

        if (input.isKeyPressed(KeyboardKey::F)) {
            Object obj(cube, cube.transform, "Cube");
            obj.transform.translate(vec3(0.0f, 10.0f, 0.0f));
            obj.transform.rotate(
                glm::radians(real_dist(rng)), vec3(int_dist(rng), int_dist(rng), int_dist(rng))
            );

            obj.aabb = calculateAABB(obj.scene, obj.transform * cube.transform);

            obj.rigidBodyId = physicsSystem.createBox(obj.transform, obj.aabb.getHalfExtent());
            gameState.objects.push_back(obj);
        }

        // create plane
        if (input.isKeyPressed(KeyboardKey::T)) {
            Object obj(cube, cube.transform, "Plane");
            obj.transform.scale(vec3(3.0f, 0.2f, 0.2f));
            obj.transform.translate(vec3(0.0f, 10.0f, 0.0f));
            obj.transform.rotate(
                glm::radians(real_dist(rng)), vec3(int_dist(rng), int_dist(rng), int_dist(rng))
            );

            obj.aabb = calculateAABB(obj.scene, obj.transform * cube.transform);

            obj.rigidBodyId = physicsSystem.createBox(obj.transform, obj.aabb.getHalfExtent());
            gameState.objects.push_back(obj);
        }

        // create sphere
        if (input.isKeyPressed(KeyboardKey::V)) {
            Object obj(sphere, sphere.transform, "Sphere");
            obj.transform.translate(vec3(0.0f, 10.0f, 0.0f));
            obj.aabb = calculateAABB(obj.scene, obj.transform * sphere.transform);

            obj.rigidBodyId = physicsSystem.createSphere(obj.transform, 1.0f);
            gameState.objects.push_back(obj);
        }

        // enable imgui
        if (input.isKeyPressed(KeyboardKey::H)) {
            state.imgui = !state.imgui;
        }

        // enable fullscreen
        if (input.isKeyPressed(KeyboardKey::F)) {
            // TODO: this is not working for some reason
            // state.fullscreen = !state.fullscreen;
            // SDL_SetWindowFullscreen(window, state.fullscreen);
            // SDL_SyncWindow(window);
            // renderer.requestResize();
        }

        camera.handleEvent(event, deltaTime);
        car.processInput();
    }
}

void Application::update(float deltaTime)
{
    for (auto &object : gameState.objects) {
        object.scene.updateAnimation(renderer.getGraphics(), deltaTime);
    }

    // car update should be called before physics system update
    car.update(deltaTime);
    camera.setPosition(car.getPosition());

    physicsSystem.update(deltaTime);

    for (auto &object : gameState.objects) {
        if (object.rigidBodyId > -1) {
            vec3 position = physicsSystem.getPosition(object.rigidBodyId);
            quat rotation = physicsSystem.getRotation(object.rigidBodyId);
            vec3 scale = object.transform.getScale();

            object.transform = Transform(position, rotation, scale);
        }
    }

    // update camera
    camera.update(deltaTime);
}

} // namespace rebirth