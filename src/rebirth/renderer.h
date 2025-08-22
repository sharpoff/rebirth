#pragma once

#include <rebirth/camera.h>
#include <rebirth/types.h>
#include <rebirth/vulkan/graphics.h>

namespace rebirth
{

const int shadowMapSize = 1024;

class Renderer
{
public:
    Renderer(SDL_Window *window);
    ~Renderer();

    void render();
    void requestResize() const;

    void setCamera(Camera *camera) { this->camera = camera; }

private:
    void updateDynamicData();

    void recordShadowPass(VkCommandBuffer cmd);
    void recordMeshPass(VkCommandBuffer cmd);
    void recordImGuiPass(VkCommandBuffer cmd);
    void recordSkyboxPass(VkCommandBuffer cmd);

    void createResources();
    void createBuffers();
    void createColorImage();
    void createDepthImage();
    void createShadowMap();
    void createSkybox();
    void createDescriptors();
    void createPipelines();

    VkPipelineLayout pipelineLayout;
    struct
    {
        VkPipeline scene;
        VkPipeline shadow;
        VkPipeline skybox;
    } pipelines;

    VkDescriptorPool pool;
    VkDescriptorSetLayout setLayout;
    VkDescriptorSet set;

    vulkan::Image colorImage;
    vulkan::Image depthImage;
    vulkan::Image shadowMap;
    vulkan::Image skybox;

    vulkan::Buffer uboBuffer;
    vulkan::Buffer materialsBuffer;
    vulkan::Buffer lightsBuffer;

    std::vector<vulkan::Image> textures;
    std::unordered_map<std::string, Scene> scenes;
    std::vector<Material> materials;
    std::vector<Light> lights;

    std::unordered_map<std::string, VkShaderModule> shaders;

    SDL_Window *window;
    vulkan::Graphics *graphics;
    Camera *camera;
};

} // namespace rebirth