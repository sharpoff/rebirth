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

    std::vector<vulkan::Image> images;
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector<Light> lights;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

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

    std::unordered_map<std::string, VkShaderModule> loadShaderModules(std::filesystem::path directory);

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

    std::unordered_map<std::string, VkPipeline> pipelines;
    std::unordered_map<std::string, VkPipelineLayout> pipelineLayouts;

    // Common
    Mesh cubeMesh;
    Mesh sphereMesh;
    Mesh cylinderMesh;

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

    std::vector<Vertex> debugDrawVertices;
    std::vector<MeshDraw> meshDraws;
    std::vector<uint32_t> opaqueDraws;

    VkQueryPool queryPool;
    std::array<uint64_t, 2> timestamps;

    SDL_Window *window;
    Graphics graphics;

    bool prepared = false;
};