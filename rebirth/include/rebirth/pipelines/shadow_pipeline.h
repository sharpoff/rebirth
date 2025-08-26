#pragma once

#include <vector>
#include <volk.h>

#include <rebirth/resource_manager.h>
#include <rebirth/types.h>

using namespace rebirth::vulkan;

namespace rebirth::vulkan
{
class Graphics;
}

namespace rebirth
{

class ShadowPipeline
{
public:
    void initialize(Graphics &graphics);
    void destroy(VkDevice device);

    void setShadowMapImage(Image &shadowMap) { this->shadowMap = &shadowMap; }
    uint32_t getShadowMapSize() { return shadowMapSize; }

    void beginFrame(Graphics &graphics, VkCommandBuffer cmd);
    void endFrame(Graphics &graphics, VkCommandBuffer cmd, bool debug = false);
    void draw(Graphics &graphics, ResourceManager &resourceManager, VkCommandBuffer cmd, std::vector<MeshDrawCommand> &drawCommands, mat4 lightMVP);

private:
    void debugDraw(Graphics &graphics, VkCommandBuffer cmd);

    VkPipeline pipeline;
    VkPipeline debugPipeline;
    VkPipelineLayout layout;

    struct PushConstant
    {
        mat4 transform;
        VkDeviceAddress vertexBuffer;
        int materialIdx = -1; // dummy
    };

    Image *shadowMap;
    const uint32_t shadowMapSize = 2048;
};

} // namespace rebirth