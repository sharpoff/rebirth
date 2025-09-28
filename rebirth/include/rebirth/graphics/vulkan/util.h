#pragma once

#include <string>

#include <assert.h>
#include <filesystem>
#include <vector>
#include <volk.h>

#define VK_CHECK(code)                                                                             \
    do {                                                                                           \
        VkResult res = (code);                                                                     \
        assert(res == VK_SUCCESS);                                                                 \
    } while (0)

namespace vulkan::util
{

VkShaderModule loadShaderModule(VkDevice device, std::filesystem::path path);

void setDebugName(
    VkDevice device,
    uint64_t objectHandle,
    VkObjectType objectType,
    std::string name
);
void beginDebugLabel(VkCommandBuffer cmd, const char *name, float color[4]);
void endDebugLabel(VkCommandBuffer cmd);

void setViewport(VkCommandBuffer cmd, float x, float y, float width, float height);
void setScissor(VkCommandBuffer cmd, VkExtent2D extent);

void beginRendering(
    VkCommandBuffer cmd,
    std::vector<VkRenderingAttachmentInfo> colorAttachments,
    const VkRenderingAttachmentInfo *depthAttachment,
    VkExtent2D extent
);
void endRendering(VkCommandBuffer cmd);

} // namespace vulkan
