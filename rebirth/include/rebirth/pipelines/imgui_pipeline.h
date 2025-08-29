#pragma once

#include <volk.h>

namespace rebirth::vulkan
{
class Graphics;
}

namespace rebirth
{

class ImGuiPipeline
{
public:
    void initialize(vulkan::Graphics &graphics);
    void destroy(VkDevice device);

    void beginFrame(vulkan::Graphics &graphics, VkCommandBuffer cmd);
    void endFrame(vulkan::Graphics &graphics, VkCommandBuffer cmd);
};

} // namespace rebirth