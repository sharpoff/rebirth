#pragma once

#include <volk.h>

#include <rebirth/types/mesh.h>

class SkyboxPipeline
{
public:
    void initialize(ModelID cubeModelId);
    void destroy();

    void beginFrame(VkCommandBuffer cmd);
    void endFrame(VkCommandBuffer cmd);
    void drawSkybox(VkCommandBuffer cmd, ImageID skyboxId);

private:
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;

    struct PushConstant
    {
        int skyboxId;
    };

    ModelID cubeModelId = ModelID::Invalid;
};