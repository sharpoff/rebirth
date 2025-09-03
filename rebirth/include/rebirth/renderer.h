#pragma once

#include <rebirth/pipelines/imgui_pipeline.h>
#include <rebirth/pipelines/mesh_pipeline.h>
#include <rebirth/pipelines/shadow_pipeline.h>
#include <rebirth/pipelines/skybox_pipeline.h>
#include <rebirth/pipelines/wireframe_pipeline.h>
#include <rebirth/types/scene.h>

#include <rebirth/resource_manager.h>
#include <rebirth/types/animation.h>
#include <rebirth/types/camera.h>
#include <rebirth/types/light.h>
#include <rebirth/vulkan/graphics.h>

#include <rebirth/application_state.h>

#include <rebirth/game/object.h>

using namespace rebirth::vulkan;

namespace rebirth
{

static const int maxMaterialsNum = 100;
static const int maxLightsNum = 100;

class Renderer
{
public:
    Renderer() = default;
    ~Renderer() = default;

    void initialize(SDL_Window *window);
    void shutdown();

    void addLight(Light light);

    void drawScene(Scene &scene, Transform transform);
    void drawObject(Object object);

    void present(ApplicationState &state, Camera &camera);

    void requestResize() { graphics.requestResize(); }

    vulkan::Graphics &getGraphics() { return graphics; }
    ResourceManager &getResourceManager() { return resourceManager; }

protected:
    void updateDynamicData(Camera &camera);
    void updateImGui(ApplicationState &state, Camera &camera);

    void createResources();
    void createBuffers();
    void updateDescriptors();

    MeshPipeline meshPipeline;
    ShadowPipeline shadowPipeline;
    SkyboxPipeline skyboxPipeline;
    ImGuiPipeline imguiPipeline;
    WireframePipeline wireframePipeline;

    size_t shadowMapIdx;
    size_t skyboxIdx;

    vulkan::Buffer sceneDataBuffer;
    vulkan::Buffer materialsBuffer;
    vulkan::Buffer lightsBuffer;

    ResourceManager resourceManager;
    ImageID defaultImageId;
    MaterialID defaultMaterialId;

    std::vector<DrawCommand> drawCommands;

    SDL_Window *window;
    vulkan::Graphics graphics;

    bool prepared = false;
};

} // namespace rebirth