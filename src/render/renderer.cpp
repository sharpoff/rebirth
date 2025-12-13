#include "render/renderer.h"

#include <math/math.h>
#include "core/filesystem.h"
#include "render/render_types.h"

#ifdef RENDER_API_VULKAN
#include "render/vulkan/vulkan_render_device.h"
#endif

#include <stb_image.h>

Renderer::Renderer(Application *application)
{
#ifdef RENDER_API_VULKAN
    device = eastl::make_unique<VulkanRenderDevice>(application);
#endif

    vec2 windowSize = application->getWindowSize();

    { // color target
        ImageCreateInfo createInfo = {
            .width = (uint32_t)windowSize.x,
            .height = (uint32_t)windowSize.y,
            .mipLevels = device->calculateMipLevels(windowSize.x, windowSize.y),
            .sampleCount = 1,
            .usage = IMAGE_USAGE_COLOR_ATTACHMENT,
            .format = IMAGE_FORMAT_R8G8B8A8_SRGB,
        };

        colorTarget = device->createImage(createInfo);
    }

    { // depth target
        ImageCreateInfo createInfo = {
            .width = (uint32_t)windowSize.x,
            .height = (uint32_t)windowSize.y,
            .mipLevels = 1,
            .sampleCount = 1,
            .usage = IMAGE_USAGE_DEPTH_ATTACHMENT,
            .format = IMAGE_FORMAT_D32_SFLOAT,
        };

        depthTarget = device->createImage(createInfo);
    }

    // create pipelines
    {
        const Vector<DescriptorSetLayoutBinding> bindings0 = {
            {0, DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024, SHADER_STAGE_FRAGMENT}, // images
            {1, DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, SHADER_STAGE_VERTEX}, // vertex buffer
        };

        const Vector<DescriptorSetLayout> descriptorSetLayouts = {
            {bindings0}, // set 0
        };

        PipelineLayoutCreateInfo layoutCreateInfo = {};
        layoutCreateInfo.descriptorSetLayouts = descriptorSetLayouts;

        geometryPipelineLayout = device->createPipelineLayout(layoutCreateInfo);

        const RenderPipelineCreateInfo createInfo = {
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
            {{-1.0f, 1.0f, 0.0f}, 0.0f, {1.0f, 0.0f, 0.0f}, 1.0f},
            {{1.0f, 1.0f, 0.0f}, 1.0f, {0.0f, 1.0f, 0.0f}, 1.0f},
            {{0.0f, -1.0f, 0.0f}, 0.5f, {0.0f, 0.0f, 1.0f}, 0.0f},
        };

        const BufferCreateInfo createInfo = {
            .size = vertices.size() * sizeof(SimpleVertex),
            .usage = BUFFER_USAGE_STORAGE | BUFFER_USAGE_TRANSFER_DST,
        };
        vertexBuffer = device->createBuffer(createInfo);

        device->uploadBufferData(vertexBuffer, vertices.data(), createInfo.size);
    }

    // create samplers
    {
        SamplerCreateInfo samplerCreateInfo = {};
        samplerCreateInfo.magFilter = SAMPLER_FILTER_LINEAR;
        samplerCreateInfo.minFilter = SAMPLER_FILTER_LINEAR;
        samplerCreateInfo.addressModeU = SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.addressModeV = SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.addressModeW = SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

        linearSampler = device->createSampler(samplerCreateInfo);

        samplerCreateInfo.magFilter = SAMPLER_FILTER_NEAREST;
        samplerCreateInfo.minFilter = SAMPLER_FILTER_NEAREST;
        nearestSampler = device->createSampler(samplerCreateInfo);
    }

    // load image
    {
        testImage = loadImageFromFile("assets/textures/checkerboard.png", IMAGE_USAGE_SAMPLED | IMAGE_USAGE_TRANSFER_DST);
        assert(testImage != nullptr);

        const ImageViewCreateInfo viewCreateInfo = {testImage};
        testImageView = device->createImageView(viewCreateInfo);
    }

    // write descriptor set
    {
        device->writeDescriptor(0, testImageView, linearSampler, DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
        device->writeDescriptor(1, vertexBuffer, DESCRIPTOR_TYPE_STORAGE_BUFFER);
        device->updateDescriptor(geometryPipeline->layout, 0);
    }
}

Renderer::~Renderer()
{
    device->deviceWaitIdle();

    device->destroyPipelineLayout(geometryPipelineLayout);
    device->destroyPipeline(geometryPipeline);

    device->destroySampler(linearSampler);
    device->destroySampler(nearestSampler);

    device->destroyBuffer(vertexBuffer);
    device->destroyImage(testImage);
    device->destroyImage(testImageView);
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

    device->bindPipeline(cmd, geometryPipeline);
    device->draw(cmd, vertices.size(), 1, 0, 0);

    device->endRendering(cmd);

    device->endCommandBuffer(cmd);
    device->submitCommandBuffer(cmd);
}

SharedPtr<Image> Renderer::loadImageFromFile(std::filesystem::path path, ImageUsageFlags usage)
{
    uint32_t width, height, channels;
    unsigned char *pixels = stbi_load(path.c_str(), (int*)&width, (int*)&height, (int*)&channels, STBI_rgb_alpha);
    if (!pixels) {
        LOGE("Failed to load image from file '%s'!", path.c_str());
        return nullptr;
    }

    const ImageCreateInfo createInfo = {
        .width = width,
        .height = height,
        .usage = usage,
    };

    usage |= IMAGE_USAGE_TRANSFER_DST; // it should be transfer_dst to upload into it

    SharedPtr<Image> image = device->createImage(createInfo);

    printf("width * height * channels = %d\n", width * height * channels);

    device->uploadImageData(image, pixels, width * height * channels);

    return image;
}