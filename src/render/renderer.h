#pragma once

#include "core/application.h"
#include "render/render_device.h"

#define RENDER_API_VULKAN

class Renderer
{
public:
    Renderer(Application *application);
    ~Renderer();

    void draw();

private:
    UniquePtr<RenderDevice> device;

    struct SimpleVertex
    {
        vec3 position;
        vec3 color;
    };

    SharedPtr<Image> colorTarget = nullptr;
    SharedPtr<Image> depthTarget = nullptr;

    SharedPtr<Buffer> vertexBuffer = nullptr;

    Vector<SimpleVertex> vertices;

    SharedPtr<PipelineLayout> geometryPipelineLayout = nullptr;
    SharedPtr<RenderPipeline> geometryPipeline = nullptr;
};