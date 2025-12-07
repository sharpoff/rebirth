#pragma once

#include "render/render_types.h"

// clang-format off
#include <volk.h>
#include <vk_mem_alloc.h>
// clang-format on

struct VulkanAccessInfo
{
    VkPipelineStageFlags stage = VK_PIPELINE_STAGE_NONE;
    VkAccessFlags        access = VK_ACCESS_NONE;
    VkImageLayout        layout = VK_IMAGE_LAYOUT_UNDEFINED;
};

struct VulkanAllocation
{
    VmaAllocation     handle = VK_NULL_HANDLE;
    VmaAllocationInfo info;
};

struct VulkanBuffer : public Buffer
{
    VkBuffer         buffer = VK_NULL_HANDLE;
    VulkanAllocation allocation;
    VkDeviceAddress  address; // if used
};

struct VulkanImage : public Image
{
    VkImage          image;
    VkImageView      view;
    VulkanAllocation allocation;
};

struct VulkanSampler : public Sampler
{
    VkSampler sampler;
};

struct VulkanPipelineLayout : public PipelineLayout
{
    VkPipelineLayout              layout;
    Vector<VkDescriptorSetLayout> descriptorSetLayouts; // for each descriptor set
    // XXX: this is probably not very efficient, cuz we can use similar descriptor set for multiple pipelines, not only one
    Vector<VkDescriptorSet>       descriptorSets;
};

struct VulkanRenderPipeline : public RenderPipeline
{
    VkPipeline pipeline;
};

struct VulkanComputePipeline : public ComputePipeline
{
    VkPipeline pipeline;
};

struct VulkanCommandBuffer : public CommandBuffer
{
    VkCommandBuffer cmd;
};