#pragma once

#include <limits>
#include <stdint.h>

#include "EASTL/vector.h"

enum class CompareOperator
{
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always,
};

//========================
//          Texture
//========================

enum class TextureType : uint8_t
{
    Texture1D,
    Texture2D,
    Texture3D,
    Cube,
};

enum class TextureUsageFlags : uint16_t
{
    TransferSrc = 0,
    TransferDst = 1 << 1,
    Sampled = 1 << 2,
    Storage = 1 << 3,
    ColorAttachment = 1 << 4,
    DepthAttachment = 1 << 5,
    StencilAttachment = 1 << 6,
    TransientAttachment = 1 << 7,
    InputAttachment = 1 << 8,
    HostTransfer = 1 << 9,
};
using TextureUsageMask = uint16_t;

enum class TextureFormat : uint8_t
{
    R8G8B8A8_SRGB,
    B8G8R8A8_UNORM,
    D32_SFLOAT,
};

struct Texture
{
    uint32_t         width = 0;
    uint32_t         height = 0;
    uint32_t         layerCount = 0;
    uint32_t         levelCount = 1;
    uint8_t          sampleCount = 1;
    TextureType      type = TextureType::Texture2D;
    TextureUsageMask usage = ((int)TextureUsageFlags::Sampled);
    TextureFormat    format = TextureFormat::R8G8B8A8_SRGB;
    Texture         *pViewedTexture = nullptr;
};

struct TextureCreateParams
{
    uint32_t         width = 0;
    uint32_t         height = 0;
    uint32_t         arrayLayers = 0;
    uint32_t         mipLevels = 1;
    uint8_t          sampleCount = 1;
    TextureType      type = TextureType::Texture2D;
    TextureUsageMask usage = ((int)TextureUsageFlags::Sampled);
    TextureFormat    format = TextureFormat::R8G8B8A8_SRGB;
};

struct TextureViewCreateParams
{
    Texture *pTexture;
};

//========================
//          Buffer
//========================

enum class BufferUsageFlags : uint8_t
{
    TransferSrc = 0,
    TransferDst = 1 << 1,
    Uniform = 1 << 2,
    Storage = 1 << 3,
    Index = 1 << 4,
    Vertex = 1 << 5,
    Indirect = 1 << 6,
    DeviceAddress = 1 << 7,
};
using BufferUsageMask = uint8_t;

struct Buffer
{
    uint64_t        size;
    BufferUsageMask usage;
};

struct BufferCreateParams
{
    uint64_t        size;
    BufferUsageMask usage;
};

//========================
//          Sampler
//========================

enum class SamplerFilter : uint8_t
{
    Nearest,
    Linear,
};

enum class SamplerAddressMode : uint8_t
{
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder,
    MirrorClampToEdge,
};

struct Sampler
{
    float              mipLodBias = 0.0f;
    float              minLod = 0.0f;
    float              maxLod = std::numeric_limits<float>::max();
    float              maxAnisotropy = 0.0f;
    SamplerFilter      magFilter = SamplerFilter::Linear;
    SamplerFilter      minFilter = SamplerFilter::Linear;
    SamplerFilter      mipmapMode = SamplerFilter::Linear;
    SamplerAddressMode addressModeU = SamplerAddressMode::ClampToBorder;
    SamplerAddressMode addressModeV = SamplerAddressMode::Repeat;
    SamplerAddressMode addressModeW = SamplerAddressMode::Repeat;
    CompareOperator    compareOp = CompareOperator::Never;
};

struct SamplerCreateParams
{
    float              mipLodBias = 0.0f;
    float              minLod = 0.0f;
    float              maxLod = std::numeric_limits<float>::max();
    float              maxAnisotropy = 0.0f;
    SamplerFilter      magFilter = SamplerFilter::Linear;
    SamplerFilter      minFilter = SamplerFilter::Linear;
    SamplerFilter      mipmapMode = SamplerFilter::Linear;
    SamplerAddressMode addressModeU = SamplerAddressMode::ClampToBorder;
    SamplerAddressMode addressModeV = SamplerAddressMode::Repeat;
    SamplerAddressMode addressModeW = SamplerAddressMode::Repeat;
    CompareOperator    compareOp = CompareOperator::Never;
};

//========================
//   Pipeline settings
//========================

enum class ShaderStageFlags : uint8_t
{
    Vertex = 0,
    Fragment = 1 << 1,
    TessellationControl = 1 << 2,
    TessellationEvaluation = 1 << 3,
    Compute = 1 << 4,
};
using ShaderStageMask = uint8_t;

enum class PolygonMode : uint8_t
{
    Fill,
    Line,
    Point,
};

enum class CullMode : uint8_t
{
    None,
    Front,
    Back,
    FrontAndBack,
};

enum class FrontFace : uint8_t
{
    CounterClockwise,
    Clockwise,
};

enum class PrimitiveTopology : uint8_t
{
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,
    TriangleFan,
};

enum class BlendOperator : uint8_t
{
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
    None,
};

enum class BlendFactor : uint8_t
{
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
    SrcAlphaSaturate,
};

enum class ColorComponentFlags : uint8_t
{
    Red = 0,
    Green = 1 << 1,
    Blue = 1 << 2,
    Alpha = 1 << 3,
};
using ColorComponentMask = uint8_t;

enum class DynamicStateFlags : uint8_t
{
    Viewport = 0,
    Scissor = 1 << 1,
};
using DynamicStateMask = uint8_t;

//========================
//     Descriptor
//========================

enum class DescriptorType
{
    Sampler,
    CombinedImageSampler,
    SampledImage,
    StorageImage,
    UniformTexelBuffer,
    StorageTexelBuffer,
    UniformBuffer,
    StorageBuffer,
    UniformBufferDynamic,
    StorageBufferDynamic,
    InputAttachment,
};

struct DescriptorSetLayoutBinding
{
    uint32_t        binding;
    DescriptorType  descriptorType;
    uint32_t        descriptorCount;
    ShaderStageMask stageMask;
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
    ShaderStageMask stageFlags;
    uint32_t        offset;
    uint32_t        size;
};

struct PipelineLayoutCreateParams
{
    eastl::vector<DescriptorSetLayout> descriptorSetLayouts;
    eastl::vector<PushConstantRange> pushConstantRanges;
};

struct PipelineLayout
{
    eastl::vector<DescriptorSetLayout> descriptorSetLayouts;
    eastl::vector<PushConstantRange> pushConstantRanges;
};

//========================
//    Render pipeline
//========================

struct RenderPipelineCreateParams
{
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;

    PolygonMode polygonMode = PolygonMode::Fill;
    CullMode    cullMode = CullMode::None;
    FrontFace   frontFace = FrontFace::CounterClockwise;

    uint32_t patchControlPoints = 0;

    uint8_t sampleCount = 1;

    CompareOperator depthCompareOp = CompareOperator::Always;
    bool            depthWriteEnable = false;

    eastl::vector<TextureFormat> renderTargetFormats;
    TextureFormat                depthTargetFormat = TextureFormat::D32_SFLOAT;
    BlendOperator                colorBlendOp = BlendOperator::None;
    BlendFactor                  colorBlendFactorSrc = BlendFactor::Zero;
    BlendFactor                  colorBlendFactorDst = BlendFactor::Zero;
    BlendFactor                  alphaBlendFactorSrc = BlendFactor::Zero;
    BlendFactor                  alphaBlendFactorDst = BlendFactor::Zero;
    BlendOperator                alphaBlendOp = BlendOperator::None;
    ColorComponentMask           colorWriteMask = ((int)ColorComponentFlags::Red | (int)ColorComponentFlags::Green | (int)ColorComponentFlags::Blue | (int)ColorComponentFlags::Blue);

    DynamicStateMask dynamicState = ((int)DynamicStateFlags::Viewport | (int)DynamicStateFlags::Scissor);

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

struct ComputePipelineCreateParams
{
    PipelineLayout *pPipelineLayout = nullptr;
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