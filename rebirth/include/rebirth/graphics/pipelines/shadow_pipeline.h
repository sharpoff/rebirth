#pragma once

#include <volk.h>

#include <rebirth/resource_manager.h>
#include <rebirth/types/mesh_draw.h>

class ShadowPipeline
{
public:
    void initialize();
    void destroy();

    uint32_t getShadowMapSize() { return shadowMapSize; }

    void beginFrame(VkCommandBuffer cmd, const VkImageView shadowMap);
    void endFrame(VkCommandBuffer cmd, bool debug = false);
    void draw(VkCommandBuffer cmd, std::vector<MeshDraw> &meshDraws, mat4 lightMVP);

private:
    void debugDraw(VkCommandBuffer cmd);

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipeline debugPipeline = VK_NULL_HANDLE;
    VkPipelineLayout layout = VK_NULL_HANDLE;

    struct PushConstant
    {
        mat4 transform;
    };

    const uint32_t shadowMapSize = 2048;
};