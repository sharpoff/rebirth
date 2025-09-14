#pragma once

#include <rebirth/graphics/pipelines/imgui_pipeline.h>
#include <rebirth/graphics/pipelines/mesh_pipeline.h>
#include <rebirth/graphics/pipelines/shadow_pipeline.h>
#include <rebirth/graphics/pipelines/skybox_pipeline.h>

#include <rebirth/graphics/vulkan/graphics.h>
#include <rebirth/resource_manager.h>
#include <rebirth/types/animation.h>
#include <rebirth/types/camera.h>
#include <rebirth/types/light.h>
#include <rebirth/types/scene.h>

using namespace vulkan;

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
    void drawModel(ModelID modelId, Transform transform);
    void drawMesh(GPUMeshID gpuMeshId, Transform transform);

    void present(Camera &camera);

    void requestResize() { g_graphics.requestResize(); }

    void reloadShaders();

    // common mesh primitives
    ModelID getCubePrimitive() { return cubeModelId; };
    ModelID getSpherePrimitive() { return sphereModelId; };
    ModelID getCylinderPrimitive() { return cylinderModelId; };

    float getTimestampDeltaMs() { return float(timestamps[1] - timestamps[0]) * g_graphics.getDevicePropertices().limits.timestampPeriod * 1e-6; };

protected:
    void createPipelines();
    void updateDynamicData(Camera &camera);

    void createResources();
    void createBuffers();
    void updateDescriptors();

    MeshPipeline meshPipeline;
    ShadowPipeline shadowPipeline;
    SkyboxPipeline skyboxPipeline;
    ImGuiPipeline imguiPipeline;

    ImageID shadowMapId;
    ImageID skyboxId;

    vulkan::Buffer sceneDataBuffer;
    vulkan::Buffer materialsBuffer;
    vulkan::Buffer lightsBuffer;

    VkQueryPool queryPool;
    std::array<uint64_t, 2> timestamps;

    ImageID defaultImageId;
    MaterialID defaultMaterialId;

    // common primitives
    ModelID cubeModelId;
    ModelID sphereModelId;
    ModelID cylinderModelId;

    std::vector<MeshDraw> meshDraws;

    SDL_Window *window;

    bool prepared = false;
};