#pragma once

// clang-format off
#include <volk.h>
#include <vk_mem_alloc.h>
// clang-format on

namespace rebirth::vulkan
{

struct Image
{
    VkImage image = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    uint32_t mipLevels = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t channels = 0;

    VmaAllocation allocation;
    VmaAllocationInfo info;
};

struct Buffer
{
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize size = 0;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocationInfo info;
    VkDeviceAddress address;
};

} // namespace rebirth::vulkan
