#include "render/render_pass.h"

void RenderPass::read(const char *name, Texture *renderTarget)
{
}

void RenderPass::write(const char *name)
{
}

void RenderPass::setBuildCallback(eastl::function<void(CommandBuffer &commandBuffer)> &build)
{
    buildCallback = build;
}

void RenderPass::setExecuteCallback(eastl::function<void(CommandBuffer &commandBuffer)> &execute)
{
    executeCallback = execute;
}