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

using namespace rebirth::vulkan;

namespace rebirth
{

static const int maxMaterialsNum = 100;
static const int maxLightsNum = 100;

class Renderer
{
public:
    Renderer(SDL_Window *window);
    ~Renderer();

    void addLight(Light light);

    void drawScene(Scene &scene);
    void present();

    void requestResize() const { graphics->requestResize(); }
    void setCamera(Camera *camera) { this->camera = camera; }

    vulkan::Graphics &getGraphics() { return *graphics; }
    ResourceManager &getResourceManager() { return resourceManager; }

private:
    void updateDynamicData();
    void updateImGui();

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

    std::vector<DrawCommand> drawCommands;

    SDL_Window *window;
    vulkan::Graphics *graphics;
    Camera *camera;

    bool prepared = false;

    bool debugShadows = false;
    bool wireframe = false;
};

} // namespace rebirth