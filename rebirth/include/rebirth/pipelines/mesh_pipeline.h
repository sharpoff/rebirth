#pragma once

#include <vector>
#include <volk.h>

#include <rebirth/math/frustum.h>
#include <rebirth/resource_manager.h>
#include <rebirth/types/camera.h>
#include <rebirth/types/mesh.h>

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
    void initialize(vulkan::Graphics &graphics);
    void destroy(VkDevice device);

    void beginFrame(vulkan::Graphics &graphics, VkCommandBuffer cmd);
    void endFrame(vulkan::Graphics &graphics, VkCommandBuffer cmd);
    void draw(
        vulkan::Graphics &graphics,
        ResourceManager &resourceManager,
        VkCommandBuffer cmd,
        Frustum &frustum,
        std::vector<DrawCommand> &drawCommands
    );

private:
    VkPipeline pipeline;
    VkPipelineLayout layout;

    struct PushConstant
    {
        mat4 transform;
        VkDeviceAddress vertexBuffer;
        VkDeviceAddress jointMatricesBuffer;
        int materialIdx;
    };
};

} // namespace rebirth