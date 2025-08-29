#pragma once

#include <vector>
#include <volk.h>

#include <rebirth/types/mesh.h>

namespace rebirth::vulkan
{
class Graphics;
}

namespace rebirth
{

class SkyboxPipeline
{
public:
    void initialize(vulkan::Graphics &graphics);
    void destroy(vulkan::Graphics &graphics);

    void beginFrame(vulkan::Graphics &graphics, VkCommandBuffer cmd);
    void endFrame(vulkan::Graphics &graphics, VkCommandBuffer cmd);
    void drawSkybox(vulkan::Graphics &graphics, VkCommandBuffer cmd, int skyboxIndex);

private:
    VkPipeline pipeline;
    VkPipelineLayout layout;

    vulkan::Buffer vertexBuffer;
    vulkan::Buffer indexBuffer;

    struct PushConstant
    {
        VkDeviceAddress vertexBuffer;
        int skyboxIndex;
    };

    // hardcoded cube vertices and indices
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

} // namespace rebirth
