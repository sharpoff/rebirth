#pragma once

#include "render/graphics_types.h"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

struct VulkanAllocation
{
    VmaAllocation handle = VK_NULL_HANDLE;
    VmaAllocationInfo info;
};

struct VulkanBuffer : public Buffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VulkanAllocation allocation;
    VkDeviceAddress address; // if used
};

struct VulkanTexture : public Texture
{
    VkImage image;
    VkImageView view;
    VulkanAllocation allocation;
};

struct VulkanSampler : public Sampler
{
    VkSampler sampler;
};

struct VulkanPipelineLayout : public PipelineLayout
{
    VkPipelineLayout layout;
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