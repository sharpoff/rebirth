#pragma once

#include "core/camera.h"
#include "core/mesh_draw.h"
#include "core/scene_draw_data.h"
#include "editor/editor.h"
#include "graphics/vulkan/graphics.h"

#include "EASTL/string.h"

using namespace vulkan;

static const int      MAX_MATERIALS         = 100;
static const int      MAX_LIGHTS            = 100;
static const uint32_t SHADOW_MAP_SIZE       = 2048;
static const uint32_t MAX_INDIRECT_COMMANDS = 100000;

class Entity;
class Physics;

class Renderer
{
public:
    Renderer()  = default;
    ~Renderer() = default;

    void initialize(SDL_Window *window, EngineStats *engineStats, Physics *physics);
    void shutdown();

    void drawEntity(const Entity &entity, const uint32_t &drawMask = DrawMask::Opaque);
    void drawMesh(const int32_t &meshId, const uint32_t &drawMask = DrawMask::Opaque, const mat4 &transform = mat4(1.0f), const int32_t &overrideMaterialId = -1);
    void addMeshDraw(const MeshDraw &meshDraw);

    void present(Editor *editor);

    void requestResize() { graphics.requestResize(); }

    void reloadShaders();

    float     getTimestampDeltaMs() { return float(timestamps[1] - timestamps[0]) * graphics.getDevicePropertices().limits.timestampPeriod * 1e-6; };
    Graphics &getGraphics() { return graphics; };

    void setCamera(Camera *camera) { this->camera = camera; };

    void createResources(); // call after all resources loaded

private:
    void updateDynamicData();
    void loadDefaultResources();

    // Passes
    void clearPass(const VkCommandBuffer cmd);
    void shadowPass(const VkCommandBuffer cmd);
    void meshPass(const VkCommandBuffer cmd);
    void overlayPass(const VkCommandBuffer cmd);
    void imGuiPass(const VkCommandBuffer cmd, Editor *editor);
    void skyboxPass(const VkCommandBuffer cmd);

    void cullAndIndexMeshDraws();
    void sortMeshDraws(eastl::vector<uint32_t> &draws);

    eastl::unordered_map<eastl::string, VkShaderModule> loadShaderModules(std::filesystem::path directory);

    void createPipelines();
    void destroyPipelines();

    void createBuffers();
    void updateDescriptorSet();

    struct MeshPassPC
    {
        mat4 transform;
        int  materialIndex;
        unsigned int  drawMask;
    };

    struct ShadowPassPC
    {
        mat4 transform;
    };

    struct SkyboxPassPC
    {
        int skyboxIndex;
    };

    eastl::unordered_map<eastl::string, VkPipeline>       pipelines;
    eastl::unordered_map<eastl::string, VkPipelineLayout> pipelineLayouts;

    // Resources
    SceneDrawData sceneData;

    vulkan::Buffer sceneDataBuffer;
    vulkan::Buffer materialsBuffer;
    vulkan::Buffer lightsBuffer;
    vulkan::Buffer vertexBuffer;
    vulkan::Buffer indexBuffer;
    vulkan::Buffer jointMatricesBuffer;

    eastl::vector<MeshDraw> meshDraws;

    eastl::vector<uint32_t> meshDrawIndices;

    VkQueryPool               queryPool;
    eastl::array<uint64_t, 2> timestamps;

    SDL_Window *window;
    Graphics    graphics;

    EngineStats *engineStats;
    Camera *camera;
};