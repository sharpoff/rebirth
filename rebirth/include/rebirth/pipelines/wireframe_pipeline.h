
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

class WireframePipeline
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
        std::vector<DrawCommand> &drawCommands,
        vec4 color
    );

private:
    VkPipeline pipeline;
    VkPipelineLayout layout;

    struct alignas(16) PushConstant
    {
        mat4 transform;
        vec4 color;
        VkDeviceAddress vertexBuffer;
    };
};

} // namespace rebirth
