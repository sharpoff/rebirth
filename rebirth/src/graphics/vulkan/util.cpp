#include <rebirth/graphics/vulkan/util.h>

#include <rebirth/util/filesystem.h>
#include <rebirth/util/logger.h>

namespace vulkan
{
    VkShaderModule loadShaderModule(VkDevice device, std::filesystem::path path)
    {
        std::vector<char> spirv = filesystem::readFile(path);
        if (spirv.empty()) {
            logger::logInfo("Failed to read spirv file: ", path);
            return VK_NULL_HANDLE;
        }

        VkShaderModuleCreateInfo shaderModuleInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleInfo.pCode = (uint32_t *)spirv.data();
        shaderModuleInfo.codeSize = spirv.size();

        VkShaderModule module;
        VK_CHECK(vkCreateShaderModule(device, &shaderModuleInfo, nullptr, &module));
        return module;
    }

    void setDebugName(VkDevice device, uint64_t objectHandle, VkObjectType objectType, std::string name)
    {
#ifndef NDEBUG
        VkDebugUtilsObjectNameInfoEXT objectNameInfo = {
            VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
        objectNameInfo.objectHandle = objectHandle;
        objectNameInfo.objectType = objectType;
        objectNameInfo.pObjectName = name.c_str();

        vkSetDebugUtilsObjectNameEXT(device, &objectNameInfo);
#endif
    }

    void beginDebugLabel(VkCommandBuffer cmd, const char *name, float color[4])
    {
#ifndef NDEBUG
        VkDebugUtilsLabelEXT label = {VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT};
        label.pLabelName = name;
        label.color[0] = color[0];
        label.color[1] = color[1];
        label.color[2] = color[2];
        label.color[3] = color[3];

        vkCmdBeginDebugUtilsLabelEXT(cmd, &label);
#endif
    }

    void endDebugLabel(VkCommandBuffer cmd)
    {
#ifndef NDEBUG
        vkCmdEndDebugUtilsLabelEXT(cmd);
#endif
    }

    void setViewport(VkCommandBuffer cmd, float x, float y, float width, float height)
    {
        VkViewport viewport = {};
        viewport.x = x;
        viewport.y = y;
        viewport.width = width;
        viewport.height = height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &viewport);
    }

    void setScissor(VkCommandBuffer cmd, VkExtent2D extent)
    {
        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = extent;
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void beginRendering(
        VkCommandBuffer cmd,
        std::vector<VkRenderingAttachmentInfo> colorAttachments,
        const VkRenderingAttachmentInfo *depthAttachment,
        VkExtent2D extent)
    {
        VkRenderingInfo renderingInfo = {VK_STRUCTURE_TYPE_RENDERING_INFO};
        renderingInfo.colorAttachmentCount = colorAttachments.size();
        renderingInfo.pColorAttachments = colorAttachments.data();
        renderingInfo.pDepthAttachment = depthAttachment;
        renderingInfo.renderArea.extent = extent;
        renderingInfo.renderArea.offset = {0};
        renderingInfo.layerCount = 1;

        vkCmdBeginRendering(cmd, &renderingInfo);
    }

    void endRendering(VkCommandBuffer cmd) { vkCmdEndRendering(cmd); }

} // namespace vulkan::
