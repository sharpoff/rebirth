#pragma once

#include "render/render_types.h"

class RenderDevice
{
public:
    virtual ~RenderDevice() = default;

    virtual SharedPtr<Buffer>          createBuffer(const BufferCreateInfo &createInfo) = 0;
    virtual SharedPtr<Image>           createImage(const ImageCreateInfo &createInfo) = 0;
    virtual SharedPtr<Image>           createImageView(const ImageViewCreateInfo &createInfo) = 0;
    virtual SharedPtr<Sampler>         createSampler(const SamplerCreateInfo &createInfo) = 0;
    virtual SharedPtr<PipelineLayout>  createPipelineLayout(const PipelineLayoutCreateInfo &createInfo) = 0;
    virtual SharedPtr<RenderPipeline>  createRenderPipeline(const RenderPipelineCreateInfo &createInfo) = 0;
    virtual SharedPtr<ComputePipeline> createComputePipeline(const ComputePipelineCreateInfo &createInfo) = 0;

    virtual void destroyBuffer(SharedPtr<Buffer> buffer) = 0;
    virtual void destroyImage(SharedPtr<Image> image) = 0;
    virtual void destroySampler(SharedPtr<Sampler> sampler) = 0;
    virtual void destroyPipelineLayout(SharedPtr<PipelineLayout> layout) = 0;
    virtual void destroyPipeline(SharedPtr<RenderPipeline> pipeline) = 0;
    virtual void destroyPipeline(SharedPtr<ComputePipeline> pipeline) = 0;

    virtual void uploadBufferData(SharedPtr<Buffer> buffer, void *data, size_t size) = 0;
    virtual void uploadImageData(SharedPtr<Image> image, void *data, size_t size) = 0;

    virtual SharedPtr<CommandBuffer> beginCommandBuffer() = 0;
    virtual void           endCommandBuffer(SharedPtr<CommandBuffer> commandBuffer) = 0;
    virtual void           submitCommandBuffer(SharedPtr<CommandBuffer> commandBuffer) = 0;
    virtual void           draw(SharedPtr<CommandBuffer> commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
    virtual void           drawIndexed(SharedPtr<CommandBuffer> commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;
    virtual void           bindPipeline(SharedPtr<CommandBuffer> commandBuffer, SharedPtr<RenderPipeline> pipeline) = 0;
    virtual void           bindPipeline(SharedPtr<CommandBuffer> commandBuffer, SharedPtr<ComputePipeline> pipeline) = 0;
    virtual void           bindVertexBuffer(SharedPtr<CommandBuffer> commandBuffer, SharedPtr<Buffer> vertexBuffer) = 0;
    virtual void           beginRendering(SharedPtr<CommandBuffer> commandBuffer, const RenderingInfo &renderInfo) = 0;
    virtual void           endRendering(SharedPtr<CommandBuffer> commandBuffer) = 0;
    virtual void           setViewport(SharedPtr<CommandBuffer> commandBuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
    virtual void           setScissor(SharedPtr<CommandBuffer> commandBuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

    virtual void writeDescriptor(uint32_t binding, SharedPtr<Buffer> buffer, DescriptorType type, uint32_t dstArrayElement = 0) = 0;
    virtual void writeDescriptor(uint32_t binding, SharedPtr<Image> imageView, SharedPtr<Sampler> sampler, DescriptorType type, uint32_t dstArrayElement = 0) = 0;
    virtual void updateDescriptor(SharedPtr<PipelineLayout> layout, uint32_t set) = 0;

    virtual void deviceWaitIdle() = 0;

    virtual SharedPtr<Image> getSwapchainImage() = 0;

    inline uint32_t calculateMipLevels(uint32_t width, uint32_t height) const
    {
        return floor(log2(std::max(width, height))) + 1;
    }
};