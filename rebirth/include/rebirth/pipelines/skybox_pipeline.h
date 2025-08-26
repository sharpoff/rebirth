#pragma once

#include <vector>
#include <volk.h>

#include <rebirth/types.h>

using namespace rebirth::vulkan;

namespace rebirth::vulkan
{
class Graphics;
}

namespace rebirth
{

class SkyboxPipeline
{
public:
    void initialize(Graphics &graphics);
    void destroy(Graphics &graphics);

    void beginFrame(Graphics &graphics, VkCommandBuffer cmd);
    void endFrame(Graphics &graphics, VkCommandBuffer cmd);
    void drawSkybox(Graphics &graphics, VkCommandBuffer cmd, int skyboxIndex);

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
    std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.5f}, 0.0f, {0.0f, 0.0f, 1.0f}, 0.0f},  {{0.5f, -0.5f, 0.5f}, 1.0f, {0.0f, 0.0f, 1.0f}, 0.0f},    {{0.5f, 0.5f, 0.5f}, 1.0f, {0.0f, 0.0f, 1.0f}, 1.0f},
        {{-0.5f, 0.5f, 0.5f}, 0.0f, {0.0f, 0.0f, 1.0f}, 1.0f},   {{-0.5f, -0.5f, -0.5f}, 1.0f, {0.0f, 0.0f, -1.0f}, 0.0f}, {{0.5f, -0.5f, -0.5f}, 0.0f, {0.0f, 0.0f, -1.0f}, 0.0f},
        {{0.5f, 0.5f, -0.5f}, 0.0f, {0.0f, 0.0f, -1.0f}, 1.0f},  {{-0.5f, 0.5f, -0.5f}, 1.0f, {0.0f, 0.0f, -1.0f}, 1.0f},  {{0.5f, -0.5f, 0.5f}, 0.0f, {1.0f, 0.0f, 0.0f}, 0.0f},
        {{0.5f, -0.5f, -0.5f}, 1.0f, {1.0f, 0.0f, 0.0f}, 0.0f},  {{0.5f, 0.5f, -0.5f}, 1.0f, {1.0f, 0.0f, 0.0f}, 1.0f},    {{0.5f, 0.5f, 0.5f}, 0.0f, {1.0f, 0.0f, 0.0f}, 1.0f},
        {{-0.5f, -0.5f, 0.5f}, 1.0f, {-1.0f, 0.0f, 0.0f}, 0.0f}, {{-0.5f, -0.5f, -0.5f}, 0.0f, {-1.0f, 0.0f, 0.0f}, 0.0f}, {{-0.5f, 0.5f, -0.5f}, 0.0f, {-1.0f, 0.0f, 0.0f}, 1.0f},
        {{-0.5f, 0.5f, 0.5f}, 1.0f, {-1.0f, 0.0f, 0.0f}, 1.0f},  {{-0.5f, 0.5f, 0.5f}, 0.0f, {0.0f, 1.0f, 0.0f}, 0.0f},    {{0.5f, 0.5f, 0.5f}, 1.0f, {0.0f, 1.0f, 0.0f}, 0.0f},
        {{0.5f, 0.5f, -0.5f}, 1.0f, {0.0f, 1.0f, 0.0f}, 1.0f},   {{-0.5f, 0.5f, -0.5f}, 0.0f, {0.0f, 1.0f, 0.0f}, 1.0f},   {{-0.5f, -0.5f, 0.5f}, 1.0f, {0.0f, -1.0f, 0.0f}, 0.0f},
        {{0.5f, -0.5f, 0.5f}, 0.0f, {0.0f, -1.0f, 0.0f}, 0.0f},  {{0.5f, -0.5f, -0.5f}, 0.0f, {0.0f, -1.0f, 0.0f}, 1.0f},  {{-0.5f, -0.5f, -0.5f}, 1.0f, {0.0f, -1.0f, 0.0f}, 1.0f}
    };
    std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0, 4, 6, 5, 6, 4, 7, 8, 9, 10, 10, 11, 8, 12, 14, 13, 14, 12, 15, 16, 17, 18, 18, 19, 16, 20, 22, 21, 22, 20, 23};
};

} // namespace rebirth