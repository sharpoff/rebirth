#include "core/engine.h"

#include "SDL3/SDL_video.h"
#include "core/draw_mask.h"
#include "core/mesh_draw.h"
#include "core/resource_manager.h"
#include "input/input.h"
#include "graphics/gltf.h"
#include "physics/helpers.h"

#include "util/logger.h"
#include "util/util.h"

#include "imgui.h"
#include "backend/imgui_impl_sdl3.h"
#include <tracy/Tracy.hpp>

void Engine::initialize(const ApplicationInfo &appInfo)
{
    ZoneScopedN("Application init");

    timer.start();

    this->appInfo = appInfo;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LOGE("Failed to initialize SDL: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_Window *window = SDL_CreateWindow(appInfo.name.c_str(), appInfo.width, appInfo.height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        LOGE("Failed to create SDL window: %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    renderer.initialize(window, &stats);
    physics.initialize();

    // setup camera
    flyCamera.initialize(&input);
    flyCamera.setPerspectiveInf(glm::radians(60.0f), float(appInfo.width) / appInfo.height, 0.1f);
    flyCamera.setPosition(vec3(0, 2, 2));

    // add light
    ResourceManager::get()->addLight(
        GPULight{
            .type = LightType::Directional,
            .direction = vec3(0.0, -1.0, 0.0),
        });

    // load scenes
    Scene scene;
    if (!gltf::loadScene(renderer.getGraphics(), scene, "assets/models/DamagedHelmet.glb")) {
        LOGE("%s", "Failed to load scene.");
        exit(EXIT_FAILURE);
    }

    world.initialize(&physics, &input, appInfo);

    // renderer.setCamera(&flyCamera);
    // world.getPlayer().setKeyboardInput(false);
    renderer.setCamera(&world.getPlayer().getCamera());

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

    LOGI("%s", "Engine shutdown");
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

        processInput(deltaTime);
        update(deltaTime);

        if (!minimized)
            render();
    }
}

void Engine::processInput(float deltaTime)
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

        if (input.getKey(KeyboardKey::Q, InputAction::Pressed)) {
            LOGI("%s", "Reloading shaders");
            renderer.reloadShaders();
        }

        // do raycast
        if (!ImGui::GetIO().WantCaptureMouse && input.getMouseButton(MouseButton::RIGHT, InputAction::JustPressed)) {
            vec3 direction = -util::mouseToWorldDirection(vec2(event.motion.x, event.motion.y), vec2(appInfo.width, appInfo.height), flyCamera.getView(), flyCamera.getProjection());

            float raycastRange = 10000.0f;
            direction *= raycastRange;

            JPH::BodyID hitBody = physics.rayCast(MathToJolt(flyCamera.getPosition()), MathToJolt(-direction));
            if (hitBody != JPH::BodyID()) { // valid body
                Entity *entity = world.getEntityByBodyId(hitBody.GetIndexAndSequenceNumber());
                editor.selectEntity(entity);
            } else {
                editor.selectEntity(nullptr);
            }
        }

        flyCamera.processEvent(event);
        world.processEvent(event);
    }

    world.processInput(deltaTime);
}

void Engine::update(float deltaTime)
{
    ZoneScopedN("Update");

    physics.update();
    flyCamera.update(deltaTime);
    world.update(deltaTime);
    editor.update(&input, &flyCamera, appInfo);
}

void Engine::render()
{
    ZoneScopedN("Render");

    for (auto &entity : world.getEntities())
        renderer.drawEntity(entity, DrawMask::Opaque);

    for (const MeshDraw &gizmoMeshDraw : editor.getGizmoMeshDraws())
        renderer.addMeshDraw(gizmoMeshDraw);

    Player &player = world.getPlayer();
    renderer.drawMesh(player.getMeshId(), DrawMask::Opaque, player.getTransform());

    if (!renderer.present(&editor)) {
        // recreating swapchain, so change width/height in application info
        int width = 0, height = 0;
        SDL_GetWindowSize(window, &width, &height);

        appInfo.width = width;
        appInfo.height = height;
    }
}