#include "render/vulkan/vulkan_helpers.h"

#include "render/graphics_types.h"
#include "core/logger.h"

#include "render/vulkan/vulkan_config.h"
#include <vulkan/vulkan_core.h>

namespace vulkan
{
    VkImageType getImageType(TextureType type)
    {
        switch (type) {
            case TextureType::Texture1D:
                return VK_IMAGE_TYPE_1D;
            case TextureType::Texture2D:
                return VK_IMAGE_TYPE_2D;
            case TextureType::Texture3D:
                return VK_IMAGE_TYPE_3D;
            case TextureType::Cube:
                return VK_IMAGE_TYPE_2D;
        }

        LOGE("Invalid texture type %d!\n", type);
        return VK_IMAGE_TYPE_MAX_ENUM;
    }

    VkImageViewType getImageViewType(TextureType type)
    {
        switch (type) {
            case TextureType::Texture1D:
                return VK_IMAGE_VIEW_TYPE_1D;
            case TextureType::Texture2D:
                return VK_IMAGE_VIEW_TYPE_2D;
            case TextureType::Texture3D:
                return VK_IMAGE_VIEW_TYPE_3D;
            case TextureType::Cube:
                return VK_IMAGE_VIEW_TYPE_CUBE;
        }

        LOGE("Invalid texture type %d!\n", type);
        return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
    }

    VkImageSubresourceRange getImageSubresourceRange(Texture *texture)
    {
        assert(texture);

        VkImageSubresourceRange subresourceRange = {};
        if ((int)texture->usage & (int)TextureUsageFlags::DepthAttachment) {
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        } else if ((int)texture->usage & (int)TextureUsageFlags::StencilAttachment) {
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        } else {
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        subresourceRange.levelCount = texture->levelCount;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = texture->layerCount;

        return subresourceRange;
    }

    VkImageUsageFlags getImageUsageFlags(TextureUsageMask usage)
    {
        VkImageUsageFlags result = 0;
        if (usage & (int)TextureUsageFlags::TransferSrc) {
            result |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }

        if (usage & (int)TextureUsageFlags::TransferDst) {
            result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }

        if (usage & (int)TextureUsageFlags::Sampled) {
            result |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        if (usage & (int)TextureUsageFlags::Storage) {
            result |= VK_IMAGE_USAGE_STORAGE_BIT;
        }

        if (usage & (int)TextureUsageFlags::ColorAttachment) {
            result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }

        if (usage & (int)TextureUsageFlags::DepthAttachment) {
            result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }

        if (usage & (int)TextureUsageFlags::StencilAttachment) {
            result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }

        if (usage & (int)TextureUsageFlags::TransientAttachment) {
            result |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        }

        if (usage & (int)TextureUsageFlags::InputAttachment) {
            result |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        }

        if (usage & (int)TextureUsageFlags::HostTransfer) {
            result |= VK_IMAGE_USAGE_HOST_TRANSFER_BIT;
        }

        return result;
    }

    VkFormat getFormat(TextureFormat format)
    {
        switch (format) {
            case TextureFormat::R8G8B8A8_SRGB:
                return VK_FORMAT_R8G8B8A8_SRGB;
            case TextureFormat::B8G8R8A8_UNORM:
                return VK_FORMAT_B8G8R8A8_UNORM;
            case TextureFormat::D32_SFLOAT:
                return VK_FORMAT_D32_SFLOAT;
        }

        LOGE("Invalid format %d!\n", format);
        return VK_FORMAT_MAX_ENUM;
    }

    VkBufferUsageFlags getBufferUsageFlags(BufferUsageMask usage)
    {
        VkBufferUsageFlags result = 0;
        if (usage & (int)BufferUsageFlags::TransferSrc) {
            result |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }

        if (usage & (int)BufferUsageFlags::TransferDst) {
            result |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }

        if (usage & (int)BufferUsageFlags::Uniform) {
            result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        }

        if (usage & (int)BufferUsageFlags::Storage) {
            result |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        }

        if (usage & (int)BufferUsageFlags::Index) {
            result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }

        if (usage & (int)BufferUsageFlags::Vertex) {
            result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        }

        if (usage & (int)BufferUsageFlags::Indirect) {
            result |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        }

        if (usage & (int)BufferUsageFlags::DeviceAddress) {
            result |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        }

        return result;
    }

    VkFilter getFilter(SamplerFilter filter)
    {
        switch (filter) {
            case SamplerFilter::Linear:
                return VK_FILTER_LINEAR;
            case SamplerFilter::Nearest:
                return VK_FILTER_NEAREST;
        }

        LOGE("Invalid sampler filter %d!\n", filter);
        return VK_FILTER_MAX_ENUM;
    }

    VkSamplerMipmapMode getSamplerMipmapMode(SamplerFilter mode)
    {
        switch (mode) {
            case SamplerFilter::Nearest:
                return VK_SAMPLER_MIPMAP_MODE_NEAREST;
            case SamplerFilter::Linear:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }

        LOGE("Invalid sampler mipmap mode %d!\n", mode);
        return VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
    }

    VkSamplerAddressMode getSamplerAddressMode(SamplerAddressMode samplerAddressMode)
    {
        switch (samplerAddressMode) {
            case SamplerAddressMode::Repeat:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case SamplerAddressMode::MirroredRepeat:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case SamplerAddressMode::ClampToEdge:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case SamplerAddressMode::ClampToBorder:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            case SamplerAddressMode::MirrorClampToEdge:
                return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        }

        LOGE("Invalid sampler address mode %d!\n", samplerAddressMode);
        return VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
    }

    VkSampleCountFlagBits getSampleCount(uint8_t sampleCount)
    {
        switch (sampleCount) {
            case 1:
                return VK_SAMPLE_COUNT_1_BIT;
            case 2:
                return VK_SAMPLE_COUNT_2_BIT;
            case 4:
                return VK_SAMPLE_COUNT_4_BIT;
            case 8:
                return VK_SAMPLE_COUNT_8_BIT;
            case 16:
                return VK_SAMPLE_COUNT_16_BIT;
            case 32:
                return VK_SAMPLE_COUNT_32_BIT;
            case 64:
                return VK_SAMPLE_COUNT_64_BIT;
        }

        LOGE("Invalid sample count %d!\n", sampleCount);
        return VK_SAMPLE_COUNT_FLAG_BITS_MAX_ENUM;
    }

    VkCompareOp getCompareOp(CompareOperator op)
    {
        switch (op) {
            case CompareOperator::Never:
                return VK_COMPARE_OP_NEVER;
            case CompareOperator::Less:
                return VK_COMPARE_OP_LESS;
            case CompareOperator::Equal:
                return VK_COMPARE_OP_EQUAL;
            case CompareOperator::LessOrEqual:
                return VK_COMPARE_OP_LESS_OR_EQUAL;
            case CompareOperator::Greater:
                return VK_COMPARE_OP_GREATER;
            case CompareOperator::NotEqual:
                return VK_COMPARE_OP_NOT_EQUAL;
            case CompareOperator::GreaterOrEqual:
                return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case CompareOperator::Always:
                return VK_COMPARE_OP_ALWAYS;
        }

        LOGE("Invalid compare operator %d!\n", op);
        return VK_COMPARE_OP_MAX_ENUM;
    }

    VkPrimitiveTopology getPrimitiveTopology(PrimitiveTopology topology)
    {
        switch (topology) {
            case PrimitiveTopology::PointList:
                return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            case PrimitiveTopology::LineList:
                return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case PrimitiveTopology::LineStrip:
                return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            case PrimitiveTopology::TriangleList:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case PrimitiveTopology::TriangleStrip:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            case PrimitiveTopology::TriangleFan:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        }

        LOGE("Invalid primitive topology %d!\n", topology);
        return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
    }

    VkPolygonMode getPolygonMode(PolygonMode polygonMode)
    {
        switch (polygonMode) {
            case PolygonMode::Fill:
                return VK_POLYGON_MODE_FILL;
            case PolygonMode::Line:
                return VK_POLYGON_MODE_LINE;
            case PolygonMode::Point:
                return VK_POLYGON_MODE_POINT;
        }

        LOGE("Invalid polygon mode %d!\n", polygonMode);
        return VK_POLYGON_MODE_MAX_ENUM;
    }

    VkCullModeFlags getCullMode(CullMode cullMode)
    {
        switch (cullMode) {
            case CullMode::None:
                return VK_CULL_MODE_NONE;
            case CullMode::Front:
                return VK_CULL_MODE_FRONT_BIT;
            case CullMode::Back:
                return VK_CULL_MODE_BACK_BIT;
            case CullMode::FrontAndBack:
                return VK_CULL_MODE_FRONT_AND_BACK;
        }

        LOGE("Invalid cull mode %d!\n", cullMode);
        return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
    }

    VkFrontFace getFrontFace(FrontFace frontFace)
    {
        switch (frontFace) {
            case FrontFace::CounterClockwise:
                return VK_FRONT_FACE_COUNTER_CLOCKWISE;
            case FrontFace::Clockwise:
                return VK_FRONT_FACE_CLOCKWISE;
        }

        LOGE("Invalid front mode %d!\n", frontFace);
        return VK_FRONT_FACE_MAX_ENUM;
    }

    VkBlendOp getBlendOp(BlendOperator blendOp)
    {
        switch (blendOp) {
            case BlendOperator::Add:
                return VK_BLEND_OP_ADD;
            case BlendOperator::Subtract:
                return VK_BLEND_OP_SUBTRACT;
            case BlendOperator::ReverseSubtract:
                return VK_BLEND_OP_REVERSE_SUBTRACT;
            case BlendOperator::Min:
                return VK_BLEND_OP_MIN;
            case BlendOperator::Max:
                return VK_BLEND_OP_MAX;
            case BlendOperator::None:
                return VK_BLEND_OP_ADD;
        }

        LOGE("Invalid blend operator %d!\n", blendOp);
        return VK_BLEND_OP_MAX_ENUM;
    }

    VkBlendFactor getBlendFactor(BlendFactor blendFactor)
    {
        switch (blendFactor) {
            case BlendFactor::Zero:
                return VK_BLEND_FACTOR_ZERO;
            case BlendFactor::One:
                return VK_BLEND_FACTOR_ONE;
            case BlendFactor::SrcColor:
                return VK_BLEND_FACTOR_SRC_COLOR;
            case BlendFactor::OneMinusSrcColor:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            case BlendFactor::DstColor:
                return VK_BLEND_FACTOR_DST_COLOR;
            case BlendFactor::OneMinusDstColor:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            case BlendFactor::SrcAlpha:
                return VK_BLEND_FACTOR_SRC_ALPHA;
            case BlendFactor::OneMinusSrcAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case BlendFactor::DstAlpha:
                return VK_BLEND_FACTOR_DST_ALPHA;
            case BlendFactor::OneMinusDstAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            case BlendFactor::ConstantColor:
                return VK_BLEND_FACTOR_CONSTANT_COLOR;
            case BlendFactor::OneMinusConstantColor:
                return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
            case BlendFactor::ConstantAlpha:
                return VK_BLEND_FACTOR_CONSTANT_ALPHA;
            case BlendFactor::OneMinusConstantAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
            case BlendFactor::SrcAlphaSaturate:
                return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        }

        LOGE("Invalid blend factor %d!\n", blendFactor);
        return VK_BLEND_FACTOR_MAX_ENUM;
    }

    VkColorComponentFlags getColorComponentFlags(ColorComponentMask colorComponentMask)
    {
        VkColorComponentFlags result = 0;
        if (colorComponentMask & (int)ColorComponentFlags::Red) {
            result |= VK_COLOR_COMPONENT_R_BIT;
        }

        if (colorComponentMask & (int)ColorComponentFlags::Green) {
            result |= VK_COLOR_COMPONENT_G_BIT;
        }

        if (colorComponentMask & (int)ColorComponentFlags::Blue) {
            result |= VK_COLOR_COMPONENT_B_BIT;
        }

        if (colorComponentMask & (int)ColorComponentFlags::Alpha) {
            result |= VK_COLOR_COMPONENT_A_BIT;
        }

        return result;
    }

    VkDescriptorType getDescriptorType(DescriptorType type)
    {
        switch (type) {
            case DescriptorType::Sampler:
                return VK_DESCRIPTOR_TYPE_SAMPLER;
            case DescriptorType::CombinedImageSampler:
                return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            case DescriptorType::SampledImage:
                return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            case DescriptorType::StorageImage:
                return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            case DescriptorType::UniformTexelBuffer:
                return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
            case DescriptorType::StorageTexelBuffer:
                return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
            case DescriptorType::UniformBuffer:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            case DescriptorType::StorageBuffer:
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            case DescriptorType::UniformBufferDynamic:
                return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            case DescriptorType::StorageBufferDynamic:
                return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
            case DescriptorType::InputAttachment:
                return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        }

        LOGE("Invalid descriptor type %d!\n", type);
        return VK_DESCRIPTOR_TYPE_MAX_ENUM;
    }

    VkShaderStageFlags getShaderStageFlags(ShaderStageMask stage)
    {
        VkShaderStageFlags result = 0;
        if (stage & (int)ShaderStageFlags::Vertex) {
            result |= VK_SHADER_STAGE_VERTEX_BIT;
        }

        if (stage & (int)ShaderStageFlags::Fragment) {
            result |= VK_SHADER_STAGE_FRAGMENT_BIT;
        }

        if (stage & (int)ShaderStageFlags::TessellationControl) {
            result |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        }

        if (stage & (int)ShaderStageFlags::TessellationEvaluation) {
            result |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        }

        return result;
    }

    void setDebugName(VkDevice device, VkSemaphore semaphore, eastl::string name)
    {
#ifdef ENABLE_VULKAN_DEBUG
        VkDebugUtilsObjectNameInfoEXT objectNameInfo = {
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
        objectNameInfo.objectHandle = (uint64_t)semaphore;
        objectNameInfo.objectType = VK_OBJECT_TYPE_SEMAPHORE;
        objectNameInfo.pObjectName = name.c_str();

        vkSetDebugUtilsObjectNameEXT(device, &objectNameInfo);
#endif
    }

    void beginDebugLabel(VkCommandBuffer cmd, const char *name, float color[4])
    {
#ifdef ENABLE_VULKAN_DEBUG
        VkDebugUtilsLabelEXT label = {VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT};
        label.pLabelName = name;
        label.color[0] = color[0];
        label.color[1] = color[1];
        label.color[2] = color[2];
        label.color[3] = color[3];

        vkCmdBeginDebugUtilsLabelEXT(cmd, &label);
#endif
    }

    void endDebugLabel(VkCommandBuffer cmd)
    {
#ifdef ENABLE_VULKAN_DEBUG
        vkCmdEndDebugUtilsLabelEXT(cmd);
#endif
    }
} // namespace vulkan