#pragma once

#include "render/render_device.h"
#include "render/render_pass.h"

class RenderGraph
{
public:
    RenderGraph(RenderDevice *renderDevice);
    ~RenderGraph();

    RenderPass *addPass(const char *name);

    void build();
    void run();

private:
    eastl::vector<RenderPass *> renderPasses;

    RenderDevice *device = nullptr;
};