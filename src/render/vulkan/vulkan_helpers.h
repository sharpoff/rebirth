#pragma once

#include "EASTL/string.h"
#include <assert.h>
#include <vulkan/vulkan.h>

#include "render/graphics_types.h"

#define VK_CHECK(code)                 \
    do {                               \
        VkResult res = (code);         \
        if (res != VK_SUCCESS) {       \
            assert(res != VK_SUCCESS); \
        }                              \
    } while (0)

namespace vulkan
{
    VkImageType             getImageType(TextureType type);
    VkImageViewType         getImageViewType(TextureType type);
    VkImageSubresourceRange getImageSubresourceRange(Texture *texture);
    VkImageUsageFlags       getImageUsageFlags(TextureUsageMask usage);
    VkFormat                getFormat(TextureFormat format);
    VkBufferUsageFlags      getBufferUsageFlags(BufferUsageMask usage);
    VkFilter                getFilter(SamplerFilter filter);
    VkSamplerMipmapMode     getSamplerMipmapMode(SamplerFilter mode);
    VkSamplerAddressMode    getSamplerAddressMode(SamplerAddressMode samplerAddressMode);
    VkSampleCountFlagBits   getSampleCount(uint8_t sampleCount);
    VkCompareOp             getCompareOp(CompareOperator compareOp);
    VkPrimitiveTopology     getPrimitiveTopology(PrimitiveTopology topology);
    VkPolygonMode           getPolygonMode(PolygonMode polygonMode);
    VkCullModeFlags         getCullMode(CullMode cullMode);
    VkFrontFace             getFrontFace(FrontFace frontFace);
    VkBlendOp               getBlendOp(BlendOperator blendOp);
    VkBlendFactor           getBlendFactor(BlendFactor blendFactor);
    VkColorComponentFlags   getColorComponentFlags(ColorComponentMask colorComponentMask);
    VkDescriptorType        getDescriptorType(DescriptorType type);
    VkShaderStageFlags      getShaderStageFlags(ShaderStageMask stage);

    void setDebugName(VkDevice device, VkSemaphore semaphore, eastl::string name);
    void beginDebugLabel(VkCommandBuffer cmd, const char *name, float color[4]);
    void endDebugLabel(VkCommandBuffer cmd);
} // namespace vulkan