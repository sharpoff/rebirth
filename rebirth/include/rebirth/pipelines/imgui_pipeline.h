#pragma once

#include <volk.h>

#include <rebirth/types.h>

using namespace rebirth::vulkan;

namespace rebirth::vulkan
{
class Graphics;
}

namespace rebirth
{

class ImGuiPipeline
{
public:
    void initialize(Graphics &graphics);
    void destroy(VkDevice device);

    void beginFrame(Graphics &graphics, VkCommandBuffer cmd);
    void endFrame(Graphics &graphics, VkCommandBuffer cmd);
};

} // namespace rebirth