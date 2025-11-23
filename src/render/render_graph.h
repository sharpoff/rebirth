#pragma once

#include "EASTL/internal/function.h"
#include "render/render_pass.h"

class RenderGraph
{
public:
    RenderGraph() = default;
    ~RenderGraph() = default;

    void addRenderPass(const char *name, eastl::function<void(RenderPass &pass)> &setup, eastl::function<void(CommandBuffer *commandBuffer)> &execute);

    void build();
    void run();
};