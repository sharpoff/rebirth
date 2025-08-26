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

class MeshPipeline
{
public:
    void initialize(Graphics &graphics);
    void destroy(VkDevice device);

    void beginFrame(Graphics &graphics, VkCommandBuffer cmd);
    void endFrame(Graphics &graphics, VkCommandBuffer cmd);
    void draw(Graphics &graphics, ResourceManager &resourceManager, VkCommandBuffer cmd, std::vector<MeshDrawCommand> &drawCommands);

private:
    VkPipeline pipeline;
    VkPipelineLayout layout;

    struct PushConstant
    {
        mat4 transform;
        VkDeviceAddress vertexBuffer;
        int materialIdx;
    };
};

} // namespace rebirth