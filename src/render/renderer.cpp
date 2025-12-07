#include "render/renderer.h"

#include <math/common.h>
#include "core/filesystem.h"
#include "render/render_types.h"

#ifdef RENDER_API_VULKAN
#include "render/vulkan/vulkan_render_device.h"
#endif

Renderer::Renderer(Application *application)
{
#ifdef RENDER_API_VULKAN
    device = eastl::make_unique<VulkanRenderDevice>(application);
#endif

    vec2 windowSize = application->getWindowSize();

    { // color target
        ImageCreateInfo createInfo = {
            .width = (uint32_t)windowSize.x(),
            .height = (uint32_t)windowSize.y(),
            .arrayLayers = 0,
            .mipLevels = device->calculateMipLevels(windowSize.x(), windowSize.y()),
            .sampleCount = 1,
            .usage = IMAGE_USAGE_COLOR_ATTACHMENT,
            .format = IMAGE_FORMAT_R8G8B8A8_SRGB,
        };

        colorTarget = device->createImage(createInfo);
    }

    { // depth target
        ImageCreateInfo createInfo = {
            .width = (uint32_t)windowSize.x(),
            .height = (uint32_t)windowSize.y(),
            .arrayLayers = 0,
            .mipLevels = 1,
            .sampleCount = 1,
            .usage = IMAGE_USAGE_DEPTH_ATTACHMENT,
            .format = IMAGE_FORMAT_D32_SFLOAT,
        };

        depthTarget = device->createImage(createInfo);
    }

    // create pipelines
    {
        // const Vector<DescriptorSetLayoutBinding> bindings0 = {
        //     {0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, SHADER_STAGE_VERTEX},
        //     {1, DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, SHADER_STAGE_VERTEX},
        //     {2, DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024, VK_SHADER_STAGE_FRAGMENT_BIT},
        // };

        // const Vector<DescriptorSetLayout> descriptorSetLayouts = {{
        //     bindings0 // set 0
        // }};

        PipelineLayoutCreateInfo layoutCreateInfo = {};
        // layoutCreateInfo.descriptorSetLayouts = descriptorSetLayouts;

        geometryPipelineLayout = device->createPipelineLayout(layoutCreateInfo);

        const RenderPipelineCreateInfo createInfo = {
            .vertexBindings = {{0, sizeof(SimpleVertex), VERTEX_INPUT_RATE_VERTEX}},
            .vertexAttributes = {
                {0, 0, VERTEX_FORMAT_R32G32B32_SFLOAT, offsetof(SimpleVertex, position)},
                {1, 0, VERTEX_FORMAT_R32G32B32_SFLOAT, offsetof(SimpleVertex, color)},
            },
            .colorAttachmentFormats = {IMAGE_FORMAT_B8G8R8A8_SRGB},
            .pipelineLayout = geometryPipelineLayout,
            .vertexCode = filesystem::readBinaryFile("shaders/bin/triangle.vert.spv"),
            .fragmentCode = filesystem::readBinaryFile("shaders/bin/triangle.frag.spv"),
        };

        geometryPipeline = device->createRenderPipeline(createInfo);
    }

    // create vertex buffer
    {
        vertices = {
            {{-1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
            {{1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
            {{0.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        };

        const BufferCreateInfo createInfo = {
            .size = vertices.size() * sizeof(SimpleVertex),
            .usage = BUFFER_USAGE_VERTEX | BUFFER_USAGE_TRANSFER_DST,
        };
        vertexBuffer = device->createBuffer(createInfo);

        device->uploadBufferData(vertexBuffer, vertices.data(), vertices.size() * sizeof(SimpleVertex));
    }
}

Renderer::~Renderer()
{
    device->deviceWaitIdle();

    device->destroyPipelineLayout(geometryPipelineLayout);
    device->destroyPipeline(geometryPipeline);

    device->destroyBuffer(vertexBuffer);
    device->destroyImage(depthTarget);
    device->destroyImage(colorTarget);
}

void Renderer::draw()
{
    SharedPtr<CommandBuffer> cmd = device->beginCommandBuffer();

    AttachmentResource swapchainAttachment = {
        .image = device->getSwapchainImage(),
        .load = false,
        .store = true,
    };

    Vector<AttachmentResource> colorAttachments = {swapchainAttachment};

    RenderingInfo renderInfo = {};
    renderInfo.colorAttachments = colorAttachments;
    renderInfo.depthAttachment = nullptr;

    device->beginRendering(cmd, renderInfo);

    device->bindVertexBuffer(cmd, vertexBuffer);
    device->bindPipeline(cmd, geometryPipeline);
    device->draw(cmd, vertices.size(), 1, 0, 0);

    device->endRendering(cmd);

    device->endCommandBuffer(cmd);
    device->submitCommandBuffer(cmd);
}