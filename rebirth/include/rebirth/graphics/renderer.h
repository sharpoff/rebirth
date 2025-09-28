#pragma once

#include <rebirth/graphics/vulkan/graphics.h>
#include <rebirth/resource_manager.h>

#include <rebirth/types/animation.h>
#include <rebirth/types/camera.h>
#include <rebirth/types/draw_batch.h>
#include <rebirth/types/light.h>
#include <rebirth/types/mesh_draw.h>
#include <rebirth/types/scene.h>
#include <rebirth/types/scene_draw_data.h>

using namespace vulkan;

static const int maxMaterialsNum = 100;
static const int maxLightsNum = 100;
static const uint32_t shadowMapSize = 2048;

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
    void drawMesh(MeshID meshId, Transform transform);

    void present(Camera &camera);

    void requestResize() { g_graphics.requestResize(); }

    void reloadShaders();

    // common mesh primitives
    ModelID getCubePrimitive() { return cubeModelId; };
    ModelID getSpherePrimitive() { return sphereModelId; };
    ModelID getCylinderPrimitive() { return cylinderModelId; };

    float getTimestampDeltaMs() { return float(timestamps[1] - timestamps[0]) * g_graphics.getDevicePropertices().limits.timestampPeriod * 1e-6; };

protected:
    void updateDynamicData(Camera &camera);

    void drawLine(vec3 p1, vec3 p2);
    void drawPlane(vec3 p1, vec3 p2, vec3 p3, vec3 p4);
    void drawBox(vec3 pos, vec3 halfExtent);
    void drawSphere(vec3 pos, float radius);

    void shadowPass(const VkCommandBuffer cmd);
    void meshPass(const VkCommandBuffer cmd);
    void imGuiPass(const VkCommandBuffer cmd);
    void skyboxPass(const VkCommandBuffer cmd);
    void clearPass(const VkCommandBuffer cmd);

    void cullMeshDraws(mat4 viewProj);
    void sortMeshDraws(vec3 cameraPos);
    void batchMeshDraws();

    std::unordered_map<std::string, VkShaderModule> loadShaderModules(std::filesystem::path directory);

    void createPipelines();
    void destroyPipelines();

    void createResources();
    void createBuffers();
    void updateDescriptorSet();

    struct MeshPassPC
    {
        mat4 transform;
        int materialId;
    };

    struct ShadowPassPC
    {
        mat4 transform;
    };

    struct SkyboxPassPC
    {
        int skyboxId;
    };

    std::unordered_map<std::string, VkPipeline> pipelines;
    std::unordered_map<std::string, VkPipelineLayout> pipelineLayouts;

    // Common
    ModelID cubeModelId;
    ModelID sphereModelId;
    ModelID cylinderModelId;

    ImageID defaultImageId;
    MaterialID defaultMaterialId;

    ImageID shadowMapId;
    ImageID skyboxId;

    VkImageSubresourceRange colorSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};
    VkImageSubresourceRange depthSubresource = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};

    // Resources
    SceneDrawData sceneData;

    vulkan::Buffer sceneDataBuffer;
    vulkan::Buffer materialsBuffer;
    vulkan::Buffer lightsBuffer;
    vulkan::Buffer debugDrawVertexBuffer;

    std::vector<Vertex> debugDrawVertices;
    std::vector<MeshDraw> meshDraws;
    std::vector<uint32_t> opaqueDraws;
    std::vector<DrawBatch> drawBatches;
    std::vector<VkDrawIndexedIndirectCommand> drawCommands;

    VkQueryPool queryPool;
    std::array<uint64_t, 2> timestamps;

    SDL_Window *window;

    bool prepared = false;
};