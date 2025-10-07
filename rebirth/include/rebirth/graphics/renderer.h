#pragma once

#include <rebirth/graphics/vulkan/graphics.h>

#include <rebirth/core/animation.h>
#include <rebirth/core/camera.h>
#include <rebirth/core/light.h>
#include <rebirth/core/mesh_draw.h>
#include <rebirth/core/scene.h>
#include <rebirth/core/scene_draw_data.h>

using namespace vulkan;

static const int MAX_MATERIALS = 100;
static const int MAX_LIGHTS = 100;
static const uint32_t SHADOW_MAP_SIZE = 2048;
static const uint32_t MAX_INDIRECT_COMMANDS = 100000;

class Renderer
{
public:
    Renderer() = default;
    ~Renderer() = default;

    void initialize(SDL_Window *window);
    void shutdown();

    void drawScene(Scene &scene, mat4 transform = mat4(1.0f));
    void drawMesh(Mesh &mesh, mat4 transform = mat4(1.0f));

    void present(Camera &camera);

    void requestResize() { graphics.requestResize(); }

    void reloadShaders();

    float getTimestampDeltaMs() { return float(timestamps[1] - timestamps[0]) * graphics.getDevicePropertices().limits.timestampPeriod * 1e-6; };

    Graphics &getGraphics() { return graphics; };

    eastl::vector<vulkan::Image> images;
    eastl::vector<Material> materials;
    eastl::vector<Mesh> meshes;
    eastl::vector<Light> lights;

    eastl::vector<Vertex> vertices;
    eastl::vector<uint32_t> indices;

protected:
    void updateDynamicData(Camera &camera);

    void drawLine(vec3 p1, vec3 p2);
    void drawPlane(vec3 p1, vec3 p2, vec3 p3, vec3 p4);
    void drawBox(vec3 pos, vec3 halfExtent);
    void drawSphere(vec3 pos, float radius);

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
        int materialIndex;
    };

    struct ShadowPassPC
    {
        mat4 transform;
    };

    struct SkyboxPassPC
    {
        int skyboxIndex;
    };

    eastl::unordered_map<eastl::string, VkPipeline> pipelines;
    eastl::unordered_map<eastl::string, VkPipelineLayout> pipelineLayouts;

    // Common
    Primitive cubePrimitive;

    int shadowMapIndex;
    int skyboxIndex;

    // Resources
    SceneDrawData sceneData;

    vulkan::Buffer sceneDataBuffer;
    vulkan::Buffer materialsBuffer;
    vulkan::Buffer lightsBuffer;
    vulkan::Buffer vertexBuffer;
    vulkan::Buffer indexBuffer;
    vulkan::Buffer jointMatricesBuffer;

    eastl::vector<Vertex> debugDrawVertices;
    eastl::vector<MeshDraw> meshDraws;
    eastl::vector<uint32_t> opaqueDraws;

    VkQueryPool queryPool;
    eastl::array<uint64_t, 2> timestamps;

    SDL_Window *window;
    Graphics graphics;

    bool prepared = false;
    uint32_t drawCount = 0;
    float timestampDeltaMs = 0.0f;
};