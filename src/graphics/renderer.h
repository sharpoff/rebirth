#pragma once

#include "core/camera.h"
#include "core/mesh_draw.h"
#include "core/scene.h"
#include "core/scene_draw_data.h"
#include "editor/editor.h"
#include "graphics/vulkan/graphics.h"

#include "EASTL/string.h"
#include "renderdoc_app.h"

using namespace vulkan;

static const int      MAX_MATERIALS         = 100;
static const int      MAX_LIGHTS            = 100;
static const uint32_t SHADOW_MAP_SIZE       = 2048;
static const uint32_t MAX_INDIRECT_COMMANDS = 100000;

class Renderer
{
public:
    Renderer()  = default;
    ~Renderer() = default;

    void initialize(SDL_Window *window, EngineStats *engineStats);
    void shutdown();

    void drawMesh(int32_t meshId, uint32_t drawMask = DrawMask::Opaque, mat4 transform = mat4(1.0f));

    void present(Camera &camera);

    void requestResize() { graphics.requestResize(); }

    void reloadShaders();

    float     getTimestampDeltaMs() { return float(timestamps[1] - timestamps[0]) * graphics.getDevicePropertices().limits.timestampPeriod * 1e-6; };
    Graphics &getGraphics() { return graphics; };

protected:
    void updateDynamicData(Camera &camera);

    // Passes
    void shadowPass(const VkCommandBuffer cmd);
    void meshPass(const VkCommandBuffer cmd);
    void imGuiPass(const VkCommandBuffer cmd);
    void skyboxPass(const VkCommandBuffer cmd);
    void clearPass(const VkCommandBuffer cmd);

    void cullMeshDraws(mat4 viewProj);
    void sortMeshDraws(vec3 cameraPos);

    eastl::unordered_map<eastl::string, VkShaderModule> loadShaderModules(std::filesystem::path directory);

    void createPipelines();
    void destroyPipelines();

    void createResources();
    void createBuffers();
    void updateDescriptorSet();

    struct MeshPassPC
    {
        mat4 transform;
        int  materialIndex;
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

    eastl::vector<uint32_t> opaqueDraws;
    eastl::vector<uint32_t> translucentDraws;
    eastl::vector<uint32_t> shadowDraws;
    eastl::vector<uint32_t> wireframeDraws;

    Scene primitives;

    VkQueryPool               queryPool;
    eastl::array<uint64_t, 2> timestamps;

    SDL_Window *window;
    Graphics    graphics;
    Editor      editor;

    bool prepared = false;

    EngineStats *engineStats;
    RENDERDOC_API_1_1_2 *renderDocAPI;
};