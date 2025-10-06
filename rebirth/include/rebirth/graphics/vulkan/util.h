#pragma once

#include <string>

#include <assert.h>
#include <filesystem>
#include <vector>
#include <volk.h>

#define VK_CHECK(code)                      \
    do {                                    \
        VkResult res = (code);              \
        if (res != VK_SUCCESS) {            \
            assert(0 && res != VK_SUCCESS); \
        }                                   \
    } while (0)

namespace vulkan
{
    inline VkImageSubresourceRange colorSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};
    inline VkImageSubresourceRange depthSubresource = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};

    VkShaderModule loadShaderModule(VkDevice device, std::filesystem::path path);

    void setDebugName(
        VkDevice device,
        uint64_t objectHandle,
        VkObjectType objectType,
        std::string name);
    void beginDebugLabel(VkCommandBuffer cmd, const char *name, float color[4]);
    void endDebugLabel(VkCommandBuffer cmd);

    void setViewport(VkCommandBuffer cmd, float x, float y, float width, float height);
    void setScissor(VkCommandBuffer cmd, VkExtent2D extent);

    void beginRendering(
        VkCommandBuffer cmd,
        std::vector<VkRenderingAttachmentInfo> colorAttachments,
        const VkRenderingAttachmentInfo *depthAttachment,
        VkExtent2D extent);
    void endRendering(VkCommandBuffer cmd);
} // namespace vulkan
