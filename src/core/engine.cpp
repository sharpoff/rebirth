#include "core/engine.h"

#include "core/mesh_draw.h"
#include "imgui.h"
#include "input/input.h"
#include "graphics/gltf.h"
#include "util/util.h"
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

    renderer.initialize(window, &stats, &physics);
    physics.initialize();

    // setup camera
    camera.initialize(&input);
    camera.setPerspectiveInf(glm::radians(60.0f), float(width) / height, 0.1f);
    camera.setPosition(vec3(0, 2, 2));

    renderer.setCamera(&camera);

    // add light
    ResourceManager::get()->addLight(
        GPULight{
            .type = LightType::Directional,
            .direction = vec3(0.0, -1.0, 0.0),
        });

    // load scenes
    Scene scene;
    if (!gltf::loadScene(renderer.getGraphics(), scene, "assets/models/DamagedHelmet.glb")) {
        logger::logError("Failed to load scene.");
        exit(EXIT_FAILURE);
    }

    logger::logInfo("Engine initialized");

    world.initialize(&physics);
    editor.initialize(&stats);

    renderer.createResources();
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
        input.processEvent(event); // process event before everyting else
        ImGui_ImplSDL3_ProcessEvent(&event);

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

        if (event.type == SDL_EVENT_QUIT || input.getKey(KeyboardKey::ESCAPE, InputAction::JustPressed)) {
            running = false;
        }

        // if (input.getKey(KeyboardKey::R, InputAction::Pressed)) {
        //     renderer.reloadShaders();
        // }

        if (input.getKey(KeyboardKey::C, InputAction::JustReleased)) {
            logger::logInfo("Released");
        }

        // do raycast
        if (!ImGui::GetIO().WantCaptureMouse && input.getMouseButton(MouseButton::RIGHT, InputAction::JustPressed)) {
            vec3 direction = -util::mouseToWorldDirection(vec2(event.motion.x, event.motion.y), vec2(width, height), camera.getView(), camera.getProjection());

            float raycastRange = 10000.0f;
            direction *= raycastRange;

            JPH::BodyID hitBody = physics.rayCast(camera.getPosition(), -direction);
            if (hitBody != JPH::BodyID()) { // valid body
                Entity *entity = world.getEntityByBodyId(hitBody.GetIndexAndSequenceNumber());
                editor.selectEntity(entity);
            } else {
                editor.selectEntity(nullptr);
            }
        }

        camera.processEvent(event);
    }
}

void Engine::update(float deltaTime)
{
    ZoneScopedN("Update");

    world.update(deltaTime);
    physics.update(deltaTime);
    camera.update(deltaTime);
    editor.update(&input, &camera, vec2(width, height));
}

void Engine::render()
{
    ZoneScopedN("Render");

    for (auto &entity : world.getEntities())
        renderer.drawEntity(entity, DrawMask::Opaque);

    for (const MeshDraw &gizmoMeshDraw : editor.getGizmoMeshDraws())
        renderer.addMeshDraw(gizmoMeshDraw);

    renderer.present(&editor);
}