#pragma once

#include <volk.h>

class ImGuiPipeline
{
public:
    void beginFrame(VkCommandBuffer cmd);
    void draw();
    void endFrame(VkCommandBuffer cmd);
};