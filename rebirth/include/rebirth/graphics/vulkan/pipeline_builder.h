#pragma once

#include <EASTL/vector.h>
#include <volk.h>

namespace vulkan
{

class PipelineBuilder
{
public:
    PipelineBuilder();
    void clear();

    void setBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate);
    void
    setAttributeDescription(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset);
    void setPipelineLayout(VkPipelineLayout &layout);
    void clearShaders();
    void setShader(
        VkShaderModule module,
        VkShaderStageFlagBits stage,
        VkSpecializationInfo *specializationInfo = nullptr
    );
    void setPolygonMode(VkPolygonMode polygonMode);
    void setCulling(VkCullModeFlags cullMode, VkFrontFace cullFace);
    void setDepthTest(VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp compareOp = VK_COMPARE_OP_GREATER);
    void setDepthBias(bool mode);

    void setTopology(VkPrimitiveTopology topology);
    void setPatchControlPoints(uint32_t points);

    void setMultisampleCount(VkSampleCountFlagBits samples);

    VkPipeline build(VkDevice device, eastl::vector<VkFormat> colorFormats, VkFormat depthFormat = VK_FORMAT_D32_SFLOAT);

private:
    eastl::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    eastl::vector<VkVertexInputBindingDescription> bindingDescriptions;
    eastl::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    // states
    VkPipelineVertexInputStateCreateInfo vertexInputState;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
    VkPipelineTessellationStateCreateInfo tessellationState;
    VkPipelineViewportStateCreateInfo viewportState;
    VkPipelineRasterizationStateCreateInfo rasterizationState;
    VkPipelineMultisampleStateCreateInfo multisampleState;
    VkPipelineDepthStencilStateCreateInfo depthStencilState;
    VkPipelineColorBlendStateCreateInfo colorBlendState;
    VkPipelineDynamicStateCreateInfo dynamicState;
    VkPipelineLayout pipelineLayout;
};

} // namespace vulkan