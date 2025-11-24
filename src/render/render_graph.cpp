#include "render/render_graph.h"

RenderGraph::RenderGraph(RenderDevice *renderDevice)
    : device(renderDevice)
{}

RenderGraph::~RenderGraph()
{
    for (auto &renderPass : renderPasses) {
        delete renderPass;
    }
}

RenderPass *RenderGraph::addPass(const char *name)
{
    return nullptr;
}

void RenderGraph::build()
{
    // validate();

    // traverseDependencies();

    // reorderPasses();
}

void RenderGraph::run()
{
}