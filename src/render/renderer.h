#pragma once

#include "core/application.h"
#include "render/render_device.h"
#include <filesystem>

#define RENDER_API_VULKAN

class Renderer
{
public:
    Renderer(Application *application);
    ~Renderer();

    void draw();

private:
    SharedPtr<Image> loadImageFromFile(std::filesystem::path path, ImageUsageFlags usage);

    UniquePtr<RenderDevice> device;

    struct SimpleVertex
    {
        vec3 position;
        float uv_x;
        vec3 color;
        float uv_y;
    };

    SharedPtr<Image> colorTarget = nullptr;
    SharedPtr<Image> depthTarget = nullptr;

    SharedPtr<Buffer> vertexBuffer = nullptr;

    SharedPtr<Image> testImage = nullptr;
    SharedPtr<Image> testImageView = nullptr;

    SharedPtr<Sampler> linearSampler = nullptr;
    SharedPtr<Sampler> nearestSampler = nullptr;

    Vector<SimpleVertex> vertices;

    SharedPtr<PipelineLayout> geometryPipelineLayout = nullptr;
    SharedPtr<RenderPipeline> geometryPipeline = nullptr;
};