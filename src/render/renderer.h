#pragma once

#include "EASTL/unique_ptr.h"
#include "core/application.h"
#include "render/render_device.h"
#include "render/render_graph.h"

#define RENDER_API_VULKAN

class Renderer
{
public:
    Renderer(Application *application);
    ~Renderer();

    void requestResize() {}; // TODO: not implemented

private:
    Texture *colorTarget;
    Texture *depthTarget;

    eastl::unique_ptr<RenderDevice> renderDevice;
    eastl::unique_ptr<RenderGraph>  renderGraph;
};