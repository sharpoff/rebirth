#pragma once

#include <limits>
#include <stdint.h>

#include "core/stl.h"

//========================
//        General
//========================

enum CompareOp : uint8_t
{
    COMPARE_OP_NEVER,
    COMPARE_OP_LESS,
    COMPARE_OP_EQUAL,
    COMPARE_OP_LESS_OR_EQUAL,
    COMPARE_OP_GREATER,
    COMPARE_OP_NOT_EQUAL,
    COMPARE_OP_GREATER_OR_EQUAL,
    COMPARE_OP_ALWAYS,
};

enum AccessFlagBits : uint8_t
{
    ACCESS_NONE = 0,
    // read
    ACCESS_INDIRECT_COMMAND_READ,
    ACCESS_INDEX_READ,
    ACCESS_VERTEX_ATTRIBUTE_READ,
    ACCESS_UNIFORM_READ,
    ACCESS_INPUT_ATTACHMENT_READ,
    ACCESS_SHADER_READ,
    ACCESS_COLOR_ATTACHMENT_READ,
    ACCESS_DEPTH_STENCIL_ATTACHMENT_READ,
    ACCESS_TRANSFER_READ,
    ACCESS_HOST_READ,
    ACCESS_MEMORY_READ,
    // write
    ACCESS_SHADER_WRITE,
    ACCESS_COLOR_ATTACHMENT_WRITE,
    ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE,
    ACCESS_TRANSFER_WRITE,
    ACCESS_HOST_WRITE,
    ACCESS_MEMORY_WRITE,
};
using AccessFlags = uint32_t;

enum PipelineStageBits : uint8_t
{
    PIPELINE_STAGE_NONE = 0,
    PIPELINE_STAGE_DRAW_INDIRECT,
    PIPELINE_STAGE_VERTEX_INPUT,
    PIPELINE_STAGE_VERTEX_SHADER,
    PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER,
    PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER,
    PIPELINE_STAGE_GEOMETRY_SHADER,
    PIPELINE_STAGE_FRAGMENT_SHADER,
    PIPELINE_STAGE_FRAGMENT_TESTS,
    PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT,
    PIPELINE_STAGE_COMPUTE_SHADER,
    PIPELINE_STAGE_TRANSFER,
    PIPELINE_STAGE_HOST,
    PIPELINE_STAGE_ALL_COMMANDS,
};
using PipelineStageFlags = uint32_t;

//========================
//         Image
//========================

enum ImageLayout : uint8_t
{
    IMAGE_LAYOUT_UNDEFINED,
    IMAGE_LAYOUT_GENERAL,
    IMAGE_LAYOUT_OPTIMAL,
    IMAGE_LAYOUT_PRESENT,
};


enum ImageType : uint8_t
{
    IMAGE_TYPE_1D,
    IMAGE_TYPE_2D,
    IMAGE_TYPE_3D,
    IMAGE_TYPE_CUBE,
};

enum ImageUsageFlagBits : uint16_t
{
    IMAGE_USAGE_TRANSFER_SRC = 0,
    IMAGE_USAGE_TRANSFER_DST = 1 << 1,
    IMAGE_USAGE_SAMPLED = 1 << 2,
    IMAGE_USAGE_STORAGE = 1 << 3,
    IMAGE_USAGE_COLOR_ATTACHMENT = 1 << 4,
    IMAGE_USAGE_DEPTH_ATTACHMENT = 1 << 5,
    IMAGE_USAGE_STENCIL_ATTACHMENT = 1 << 6,
    IMAGE_USAGE_TRANSIENT_ATTACHMENT = 1 << 7,
    IMAGE_USAGE_INPUT_ATTACHMENT = 1 << 8,
    IMAGE_USAGE_HOST_TRANSFER = 1 << 9,
};
using ImageUsageFlags = uint32_t;

// TODO: add more formats
enum ImageFormat : uint8_t
{
    IMAGE_FORMAT_UNDEFINED,
    IMAGE_FORMAT_B8G8R8A8_SRGB,
    IMAGE_FORMAT_B8G8R8A8_UNORM,
    IMAGE_FORMAT_R8G8B8A8_SRGB,
    IMAGE_FORMAT_R8G8B8A8_UNORM,
    IMAGE_FORMAT_D32_SFLOAT,
};

struct Image
{
    uint32_t        width = 0;
    uint32_t        height = 0;
    uint32_t        layerCount = 1;
    uint32_t        levelCount = 1;
    uint8_t         sampleCount = 1;
    ImageType       type = IMAGE_TYPE_2D;
    ImageUsageFlags usage = IMAGE_USAGE_COLOR_ATTACHMENT;
    ImageFormat     format = IMAGE_FORMAT_R8G8B8A8_SRGB;
    Image          *pViewedImage = nullptr;
    bool            isSwapchain = false;
};

struct ImageCreateInfo
{
    uint32_t        width = 0;
    uint32_t        height = 0;
    uint32_t        arrayLayers = 1;
    uint32_t        mipLevels = 1;
    uint8_t         sampleCount = 1;
    ImageType       type = IMAGE_TYPE_2D;
    ImageUsageFlags usage = IMAGE_USAGE_COLOR_ATTACHMENT;
    ImageFormat     format = IMAGE_FORMAT_R8G8B8A8_SRGB;
};

struct ImageViewCreateInfo
{
    SharedPtr<Image> image;
};

//========================
//         Buffer
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
//        Sampler
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
    CompareOp          compareOp = COMPARE_OP_NEVER;
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
    CompareOp          compareOp = COMPARE_OP_NEVER;
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
    SHADER_STAGE_ALL = SHADER_STAGE_VERTEX | SHADER_STAGE_FRAGMENT | SHADER_STAGE_TESSELLATIONCONTROL | SHADER_STAGE_TESSELLATIONEVALUATION | SHADER_STAGE_COMPUTE,
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

enum BlendOp : uint8_t
{
    BLEND_OP_ADD,
    BLEND_OP_SUBTRACT,
    BLEND_OP_REVERSE_SUBTRACT,
    BLEND_OP_MIN,
    BLEND_OP_MAX,
    BLEND_OP_NONE,
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
using ColorComponentFlags = uint32_t;

enum DynamicStateFlagBits : uint8_t
{
    DYNAMIC_STATE_VIEWPORT = 0,
    DYNAMIC_STATE_SCISSOR = 1 << 1,
};
using DynamicStateFlags = uint32_t;

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
    Vector<DescriptorSetLayoutBinding> bindings;
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
    Vector<DescriptorSetLayout> descriptorSetLayouts;
    Vector<PushConstantRange>   pushConstantRanges;
};

struct PipelineLayout
{
    Vector<DescriptorSetLayout> descriptorSetLayouts;
    Vector<PushConstantRange>   pushConstantRanges;
};

//========================
//    Render pipeline
//========================

enum VertexInputRate
{
    VERTEX_INPUT_RATE_VERTEX,
    VERTEX_INPUT_RATE_INSTANCE,
};

enum VertexFormat
{
    VERTEX_FORMAT_R32_SFLOAT,
    VERTEX_FORMAT_R32G32_SFLOAT,
    VERTEX_FORMAT_R32G32B32_SFLOAT,
    VERTEX_FORMAT_R32G32B32A32_SFLOAT,
};

struct VertexBinding
{
    uint32_t        binding;
    uint32_t        stride;
    VertexInputRate inputRate;
};

struct VertexAttribute
{
    uint32_t     location;
    uint32_t     binding;
    VertexFormat format;
    uint32_t     offset;
};

struct RenderPipelineCreateInfo
{
    Vector<VertexBinding>   vertexBindings;
    Vector<VertexAttribute> vertexAttributes;

    PrimitiveTopology topology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    PolygonMode polygonMode = POLYGON_MODE_FILL;
    CullMode    cullMode = CULL_MODE_NONE;
    FrontFace   frontFace = FRONT_FACE_COUNTER_CLOCKWISE;

    uint32_t patchControlPoints = 0;

    uint8_t sampleCount = 1;

    CompareOp depthCompareOp = COMPARE_OP_ALWAYS;
    bool      depthWriteEnable = false;

    Vector<ImageFormat> colorAttachmentFormats;
    ImageFormat         depthAttachmentFormat = IMAGE_FORMAT_UNDEFINED;

    BlendOp             colorBlendOp = BLEND_OP_NONE;
    BlendFactor         colorBlendFactorSrc = BLEND_FACTOR_ZERO;
    BlendFactor         colorBlendFactorDst = BLEND_FACTOR_ZERO;
    BlendFactor         alphaBlendFactorSrc = BLEND_FACTOR_ZERO;
    BlendFactor         alphaBlendFactorDst = BLEND_FACTOR_ZERO;
    BlendOp             alphaBlendOp = BLEND_OP_NONE;
    ColorComponentFlags colorWriteMask = COLOR_COMPONENT_R | COLOR_COMPONENT_G | COLOR_COMPONENT_B | COLOR_COMPONENT_A;

    DynamicStateFlags dynamicState = DYNAMIC_STATE_VIEWPORT | DYNAMIC_STATE_SCISSOR;

    SharedPtr<PipelineLayout> pipelineLayout = nullptr;

    Vector<char> vertexCode;
    Vector<char> fragmentCode;
    Vector<char> tessellationControlCode;
    Vector<char> tessellationEvaluationCode;
};

struct RenderPipeline
{
    SharedPtr<PipelineLayout> layout = nullptr;
};

//========================
//    Compute pipeline
//========================

struct ComputePipelineCreateInfo
{
    SharedPtr<PipelineLayout>  pipelineLayout = nullptr;
    Vector<char> computeCode;
};

struct ComputePipeline
{
    SharedPtr<PipelineLayout> layout = nullptr;
};

//========================
//       Rendering
//========================

struct ImageResource
{
    SharedPtr<Image> image = nullptr;
};

struct BufferResource
{
    SharedPtr<Buffer> buffer = nullptr;
};

struct AttachmentResource
{
    SharedPtr<Image> image = nullptr;
    bool   load = false;
    bool   store = false;
};

struct RenderingInfo
{
    Vector<AttachmentResource> colorAttachments;
    AttachmentResource        *depthAttachment;
};

//========================
//    Command buffer
//========================

struct CommandBuffer
{
    // NOTE: not sure if this should be device global or command buffer specific
    Queue<RenderingInfo> renderInfos;
};