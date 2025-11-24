#pragma once

#include "render/graphics_types.h"

class RenderDevice
{
public:
    virtual ~RenderDevice() = default;

    virtual Buffer          *createBuffer(const BufferCreateInfo &createInfo) = 0;
    virtual Texture         *createTexture(const TextureCreateInfo &createInfo) = 0;
    virtual Texture         *createTextureView(const TextureViewCreateInfo &createInfo) = 0;
    virtual Sampler         *createSampler(const SamplerCreateInfo &createInfo) = 0;
    virtual PipelineLayout  *createPipelineLayout(const PipelineLayoutCreateInfo &createInfo) = 0;
    virtual RenderPipeline  *createRenderPipeline(const RenderPipelineCreateInfo &createInfo) = 0;
    virtual ComputePipeline *createComputePipeline(const ComputePipelineCreateInfo &createInfo) = 0;

    virtual void destroyBuffer(Buffer *buffer) = 0;
    virtual void destroyTexture(Texture *texture) = 0;
    virtual void destroySampler(Sampler *sampler) = 0;
    virtual void destroyPipelineLayout(PipelineLayout *layout) = 0;
    virtual void destroyPipeline(RenderPipeline *pipeline) = 0;
    virtual void destroyPipeline(ComputePipeline *pipeline) = 0;

    virtual CommandBuffer *beginCommandBuffer() = 0;
    virtual void           submitCommandBuffer(CommandBuffer *commandBuffer) = 0;
    virtual void           draw(CommandBuffer *commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
    virtual void           drawIndexed(CommandBuffer *commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) = 0;
    virtual void           bindPipeline(CommandBuffer *commandBuffer, RenderPipeline *pipeline) = 0;
    virtual void           bindPipeline(CommandBuffer *commandBuffer, ComputePipeline *pipeline) = 0;

    uint32_t calculateMipLevels(uint32_t width, uint32_t height) const
    {
        return floor(log2(std::max(width, height))) + 1;
    }
};