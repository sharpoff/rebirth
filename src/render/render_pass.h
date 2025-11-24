#pragma once

#include "render/graphics_types.h"

class RenderPass
{
public:
    void read(const char *name, Texture *renderTarget);
    void write(const char *name);

    void setBuildCallback(eastl::function<void(CommandBuffer &commandBuffer)> &build);
    void setExecuteCallback(eastl::function<void(CommandBuffer &commandBuffer)> &execute);

private:
    eastl::function<void(CommandBuffer &commandBuffer)> buildCallback;
    eastl::function<void(CommandBuffer &commandBuffer)> executeCallback;
};