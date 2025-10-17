#pragma once

// clang-format off
#include <volk.h>
#include <vk_mem_alloc.h>
// clang-format on

#include "backend/imgui_impl_vulkan.h"

namespace vulkan
{
    struct ImageCreateInfo
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint8_t channels = 4; // rgba
        VkImageUsageFlags usage = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL | VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
        VkImageType imageType = VK_IMAGE_TYPE_2D;
        VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
        VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
        VkFilter filter = VK_FILTER_LINEAR;
        VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        uint32_t arrayLayers = 1;
        VkImageCreateFlags flags = 0;
    };

    struct Image
    {
        VkImage image = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;

        uint32_t mipLevels = 1;
        uint32_t width = 0;
        uint32_t height = 0;
        uint8_t channels = 0;

        VmaAllocation allocation;
        VmaAllocationInfo info;
    };

    // used in ImGui::Image
    struct ImGuiImageWrapper
    {
        void create(const Image &image)
        {
            this->width = image.width;
            this->height = image.height;
            descriptorSet = ImGui_ImplVulkan_AddTexture(image.sampler, image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        };

        void destroy()
        {
            ImGui_ImplVulkan_RemoveTexture(descriptorSet);
        }

        ImTextureID getId()
        {
            return (ImTextureID)descriptorSet;
        }

        int32_t width = 0;
        int32_t height = 0;
        VkDescriptorSet descriptorSet;
    };

    struct BufferCreateInfo
    {
        VkDeviceSize size = 0;
        VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    };

    struct Buffer
    {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceSize size = 0;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VmaAllocationInfo info;
        VkDeviceAddress address;
    };

} // namespace vulkan
