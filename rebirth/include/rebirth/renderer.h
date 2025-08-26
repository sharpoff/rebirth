#pragma once

#include <rebirth/animation.h>
#include <rebirth/camera.h>
#include <rebirth/pipelines/imgui_pipeline.h>
#include <rebirth/pipelines/mesh_pipeline.h>
#include <rebirth/pipelines/shadow_pipeline.h>
#include <rebirth/pipelines/skybox_pipeline.h>
#include <rebirth/resource_manager.h>
#include <rebirth/types.h>
#include <rebirth/vulkan/graphics.h>

namespace rebirth
{

static const int maxMaterialsNum = 100;
static const int maxLightsNum = 100;

class Renderer
{
public:
    Renderer(SDL_Window *window);
    ~Renderer();

    // NOTE: call this after loading scenes and other resources
    void prepare();

    void render();

    void addLight(Light light);
    void addMesh(MeshIdx idx, mat4 transform, bool castShadows);

    void requestResize() const { graphics->requestResize(); }
    void setCamera(Camera *camera) { this->camera = camera; }

    Graphics &getGraphics() { return *graphics; }
    ResourceManager &getResourceManager() { return resourceManager; }

private:
    void updateDynamicData();
    void updateImGui();
    void updateAnimation();
    void updateJoints(SceneNode &node, Scene &scene);

    void createResources();
    void createBuffers();
    void createShadowMap();
    void createSkybox();
    void updateDescriptors();

    MeshPipeline meshPipeline;
    ShadowPipeline shadowPipeline;
    SkyboxPipeline skyboxPipeline;
    ImGuiPipeline imguiPipeline;

    size_t shadowMapIdx;
    size_t skyboxIdx;

    vulkan::Buffer sceneDataBuffer;
    vulkan::Buffer materialsBuffer;
    vulkan::Buffer lightsBuffer;

    ResourceManager resourceManager;

    std::vector<MeshDrawCommand> meshDrawCommands;
    std::vector<Light> lights; // per draw light

    SDL_Window *window;
    vulkan::Graphics *graphics;
    Camera *camera;

    bool prepared = false;
    bool debugShadows = false;
};

} // namespace rebirth