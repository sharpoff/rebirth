#pragma once

#include <vector>
#include <volk.h>

#include <rebirth/resource_manager.h>

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
    void initialize(vulkan::Graphics &graphics);
    void destroy(VkDevice device);

    uint32_t getShadowMapSize() { return shadowMapSize; }

    void beginFrame(vulkan::Graphics &graphics, VkCommandBuffer cmd, Image &shadowMap);
    void endFrame(vulkan::Graphics &graphics, VkCommandBuffer cmd, Image &shadowMap, bool debug = false);
    void draw(
        vulkan::Graphics &graphics,
        ResourceManager &resourceManager,
        VkCommandBuffer cmd,
        std::vector<DrawCommand> &drawCommands,
        mat4 lightMVP
    );

private:
    void debugDraw(vulkan::Graphics &graphics, VkCommandBuffer cmd);

    VkPipeline pipeline;
    VkPipeline debugPipeline;
    VkPipelineLayout layout;

    struct PushConstant
    {
        mat4 transform;
        VkDeviceAddress vertexBuffer;
        VkDeviceAddress jointMatricesBuffer;
    };

    const uint32_t shadowMapSize = 2048;
};

} // namespace rebirth