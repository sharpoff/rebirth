#pragma once

#include <limits>
#include <stdint.h>

#include "EASTL/vector.h"

enum CompareOperator
{
    COMPARE_OPERATOR_NEVER,
    COMPARE_OPERATOR_LESS,
    COMPARE_OPERATOR_EQUAL,
    COMPARE_OPERATOR_LESS_OR_EQUAL,
    COMPARE_OPERATOR_GREATER,
    COMPARE_OPERATOR_NOT_EQUAL,
    COMPARE_OPERATOR_GREATER_OR_EQUAL,
    COMPARE_OPERATOR_ALWAYS,
};

//========================
//          Texture
//========================

enum TextureType : uint8_t
{
    TEXTURE_TYPE_1D,
    TEXTURE_TYPE_2D,
    TEXTURE_TYPE_3D,
    TEXTURE_TYPE_CUBE,
};

enum TextureUsageFlagBits : uint16_t
{
    TEXTURE_USAGE_TRANSFER_SRC = 0,
    TEXTURE_USAGE_TRANSFER_DST = 1 << 1,
    TEXTURE_USAGE_SAMPLED = 1 << 2,
    TEXTURE_USAGE_STORAGE = 1 << 3,
    TEXTURE_USAGE_COLOR_ATTACHMENT = 1 << 4,
    TEXTURE_USAGE_DEPTH_ATTACHMENT = 1 << 5,
    TEXTURE_USAGE_STENCIL_ATTACHMENT = 1 << 6,
    TEXTURE_USAGE_TRANSIENT_ATTACHMENT = 1 << 7,
    TEXTURE_USAGE_INPUT_ATTACHMENT = 1 << 8,
    TEXTURE_USAGE_HOST_TRANSFER = 1 << 9,
};
using TextureUsageFlags = uint16_t;

enum TextureFormat : uint8_t
{
    TEXTURE_FORMAT_R8G8B8A8_SRGB,
    TEXTURE_FORMAT_B8G8R8A8_UNORM,
    TEXTURE_FORMAT_D32_SFLOAT,
};

struct Texture
{
    uint32_t          width = 0;
    uint32_t          height = 0;
    uint32_t          layerCount = 0;
    uint32_t          levelCount = 1;
    uint8_t           sampleCount = 1;
    TextureType       type = TEXTURE_TYPE_2D;
    TextureUsageFlags usage = TEXTURE_USAGE_COLOR_ATTACHMENT;
    TextureFormat     format = TEXTURE_FORMAT_R8G8B8A8_SRGB;
    Texture          *pViewedTexture = nullptr;
};

struct TextureCreateInfo
{
    uint32_t          width = 0;
    uint32_t          height = 0;
    uint32_t          arrayLayers = 0;
    uint32_t          mipLevels = 1;
    uint8_t           sampleCount = 1;
    TextureType       type = TEXTURE_TYPE_2D;
    TextureUsageFlags usage = TEXTURE_USAGE_COLOR_ATTACHMENT;
    TextureFormat     format = TEXTURE_FORMAT_R8G8B8A8_SRGB;
};

struct TextureViewCreateInfo
{
    Texture *pTexture;
};

//========================
//          Buffer
//========================

enum BufferUsageFlagBits : uint8_t
{
    BUFFER_USAGE_TRANSFER_SRC = 0,
    BUFFER_USAGE_TRANSFER_DST = 1 << 1,
    BUFFER_USAGE_UNIFORM = 1 << 2,
    BUFFER_USAGE_STORAGE = 1 << 3,
    BUFFER_USAGE_INDEX = 1 << 4,
    BUFFER_USAGE_VERTEX = 1 << 5,
    BUFFER_USAGE_INDIRECT = 1 << 6,
    BUFFER_USAGE_DEVICE_ADDRESS = 1 << 7,
};
using BufferUsageFlags = uint32_t;

struct Buffer
{
    uint64_t         size;
    BufferUsageFlags usage;
};

struct BufferCreateInfo
{
    uint64_t         size;
    BufferUsageFlags usage;
};

//========================
//          Sampler
//========================

enum SamplerFilter : uint8_t
{
    SAMPLER_FILTER_NEAREST,
    SAMPLER_FILTER_LINEAR,
};

enum SamplerAddressMode : uint8_t
{
    SAMPLER_ADDRESS_MODE_REPEAT,
    SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT,
    SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
    SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
};

struct Sampler
{
    float              mipLodBias = 0.0f;
    float              minLod = 0.0f;
    float              maxLod = std::numeric_limits<float>::max();
    float              maxAnisotropy = 0.0f;
    SamplerFilter      magFilter = SAMPLER_FILTER_LINEAR;
    SamplerFilter      minFilter = SAMPLER_FILTER_LINEAR;
    SamplerFilter      mipmapMode = SAMPLER_FILTER_LINEAR;
    SamplerAddressMode addressModeU = SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    SamplerAddressMode addressModeV = SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    SamplerAddressMode addressModeW = SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    CompareOperator    compareOp = COMPARE_OPERATOR_NEVER;
};

struct SamplerCreateInfo
{
    float              mipLodBias = 0.0f;
    float              minLod = 0.0f;
    float              maxLod = std::numeric_limits<float>::max();
    float              maxAnisotropy = 0.0f;
    SamplerFilter      magFilter = SAMPLER_FILTER_LINEAR;
    SamplerFilter      minFilter = SAMPLER_FILTER_LINEAR;
    SamplerFilter      mipmapMode = SAMPLER_FILTER_LINEAR;
    SamplerAddressMode addressModeU = SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    SamplerAddressMode addressModeV = SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    SamplerAddressMode addressModeW = SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    CompareOperator    compareOp = COMPARE_OPERATOR_NEVER;
};

//========================
//   Pipeline settings
//========================

enum ShaderStageFlagBits : uint8_t
{
    SHADER_STAGE_VERTEX = 0,
    SHADER_STAGE_FRAGMENT = 1 << 1,
    SHADER_STAGE_TESSELLATIONCONTROL = 1 << 2,
    SHADER_STAGE_TESSELLATIONEVALUATION = 1 << 3,
    SHADER_STAGE_COMPUTE = 1 << 4,
};
using ShaderStageFlags = uint32_t;

enum PolygonMode : uint8_t
{
    POLYGON_MODE_FILL,
    POLYGON_MODE_LINE,
    POLYGON_MODE_POINT,
};

enum CullMode : uint8_t
{
    CULL_MODE_NONE,
    CULL_MODE_FRONT,
    CULL_MODE_BACK,
    CULL_MODE_FRONT_AND_BACK,
};

enum FrontFace : uint8_t
{
    FRONT_FACE_COUNTER_CLOCKWISE,
    FRONT_FACE_CLOCKWISE,
};

enum PrimitiveTopology : uint8_t
{
    PRIMITIVE_TOPOLOGY_POINT_LIST,
    PRIMITIVE_TOPOLOGY_LINE_LIST,
    PRIMITIVE_TOPOLOGY_LINE_STRIP,
    PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
    PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
};

enum BlendOperator : uint8_t
{
    BLEND_OPERATOR_ADD,
    BLEND_OPERATOR_SUBTRACT,
    BLEND_OPERATOR_REVERSE_SUBTRACT,
    BLEND_OPERATOR_MIN,
    BLEND_OPERATOR_MAX,
    BLEND_OPERATOR_NONE,
};

enum BlendFactor : uint8_t
{
    BLEND_FACTOR_ZERO,
    BLEND_FACTOR_ONE,
    BLEND_FACTOR_SRC_COLOR,
    BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
    BLEND_FACTOR_DST_COLOR,
    BLEND_FACTOR_ONE_MINUS_DST_COLOR,
    BLEND_FACTOR_SRC_ALPHA,
    BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    BLEND_FACTOR_DST_ALPHA,
    BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
    BLEND_FACTOR_CONSTANT_COLOR,
    BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
    BLEND_FACTOR_CONSTANT_ALPHA,
    BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
    BLEND_FACTOR_SRC_ALPHA_SATURATE,
};

enum ColorComponentFlagBits : uint8_t
{
    COLOR_COMPONENT_R = 0,
    COLOR_COMPONENT_G = 1 << 1,
    COLOR_COMPONENT_B = 1 << 2,
    COLOR_COMPONENT_A = 1 << 3,
};
using ColorComponentFlags = uint8_t;

enum DynamicStateFlagBits : uint8_t
{
    DYNAMIC_STATE_VIEWPORT = 0,
    DYNAMIC_STATE_SCISSOR = 1 << 1,
};
using DynamicStateFlags = uint8_t;

//========================
//     Descriptor
//========================

enum DescriptorType
{
    DESCRIPTOR_TYPE_SAMPLER,
    DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    DESCRIPTOR_TYPE_SAMPLED_IMAGE,
    DESCRIPTOR_TYPE_STORAGE_IMAGE,
    DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
    DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
    DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    DESCRIPTOR_TYPE_STORAGE_BUFFER,
    DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
    DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
    DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
};

struct DescriptorSetLayoutBinding
{
    uint32_t         binding;
    DescriptorType   descriptorType;
    uint32_t         descriptorCount;
    ShaderStageFlags stageMask;
};

struct DescriptorSetLayout
{
    eastl::vector<DescriptorSetLayoutBinding> bindings;
};

//========================
//    Pipeline layout
//========================

struct PushConstantRange
{
    ShaderStageFlags stageFlags;
    uint32_t         offset;
    uint32_t         size;
};

struct PipelineLayoutCreateInfo
{
    eastl::vector<DescriptorSetLayout> descriptorSetLayouts;
    eastl::vector<PushConstantRange>   pushConstantRanges;
};

struct PipelineLayout
{
    eastl::vector<DescriptorSetLayout> descriptorSetLayouts;
    eastl::vector<PushConstantRange>   pushConstantRanges;
};

//========================
//    Render pipeline
//========================

struct RenderPipelineCreateInfo
{
    PrimitiveTopology topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    PolygonMode polygonMode = POLYGON_MODE_FILL;
    CullMode    cullMode = CULL_MODE_NONE;
    FrontFace   frontFace = FRONT_FACE_COUNTER_CLOCKWISE;

    uint32_t patchControlPoints = 0;

    uint8_t sampleCount = 1;

    CompareOperator depthCompareOp = COMPARE_OPERATOR_ALWAYS;
    bool            depthWriteEnable = false;

    eastl::vector<TextureFormat> renderTargetFormats;
    TextureFormat                depthTargetFormat = TEXTURE_FORMAT_D32_SFLOAT;
    BlendOperator                colorBlendOp = BLEND_OPERATOR_NONE;
    BlendFactor                  colorBlendFactorSrc = BLEND_FACTOR_ZERO;
    BlendFactor                  colorBlendFactorDst = BLEND_FACTOR_ZERO;
    BlendFactor                  alphaBlendFactorSrc = BLEND_FACTOR_ZERO;
    BlendFactor                  alphaBlendFactorDst = BLEND_FACTOR_ZERO;
    BlendOperator                alphaBlendOp = BLEND_OPERATOR_NONE;
    ColorComponentFlags          colorWriteMask = COLOR_COMPONENT_R | COLOR_COMPONENT_G | COLOR_COMPONENT_B | COLOR_COMPONENT_A;

    DynamicStateFlags dynamicState = DYNAMIC_STATE_VIEWPORT | DYNAMIC_STATE_SCISSOR;

    PipelineLayout *pPipelineLayout = nullptr;

    eastl::vector<uint32_t> vertexCode;
    eastl::vector<uint32_t> fragmentCode;
    eastl::vector<uint32_t> tessellationControlCode;
    eastl::vector<uint32_t> tessellationEvaluationCode;
};

struct RenderPipeline
{
};

//========================
//    Compute pipeline
//========================

struct ComputePipelineCreateInfo
{
    PipelineLayout         *pPipelineLayout = nullptr;
    eastl::vector<uint32_t> computeCode;
};

struct ComputePipeline
{
};

//========================
//    Command buffer
//========================

struct CommandBuffer
{
};