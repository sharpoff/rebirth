#include "rebirth/graphics/render_settings.h"
#include <rebirth/graphics/pipelines/shadow_pipeline.h>
#include <rebirth/graphics/vulkan/graphics.h>
#include <rebirth/graphics/vulkan/pipeline_builder.h>
#include <rebirth/graphics/vulkan/util.h>

#include <rebirth/util/logger.h>

using namespace vulkan;

void ShadowPipeline::initialize()
{
    const VkDevice device = g_graphics.getDevice();
    DescriptorManager &descriptorManager = g_graphics.getDescriptorManager();

    // if reloading destroy resources
    if (layout)
        vkDestroyPipelineLayout(device, layout, nullptr);
    if (pipeline)
        vkDestroyPipeline(device, pipeline, nullptr);
    if (debugPipeline)
        vkDestroyPipeline(device, debugPipeline, nullptr);

    // create pipeline layout
    VkPushConstantRange pushConstant = {
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant)
    };
    layout = g_graphics.createPipelineLayout(&descriptorManager.getSetLayout(), &pushConstant);

    const auto vertex = vulkan::util::loadShaderModule(device, "build/shaders/depth.vert.spv");

    const auto debugVert = vulkan::util::loadShaderModule(device, "build/shaders/quad.vert.spv");
    const auto debugFrag = vulkan::util::loadShaderModule(device, "build/shaders/shadow_debug.frag.spv");

    // shadow pipeline
    PipelineBuilder builder;
    builder.setPipelineLayout(layout);
    builder.setShader(vertex, VK_SHADER_STAGE_VERTEX_BIT);
    builder.setDepthTest(true);
    builder.setCulling(VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    builder.setPolygonMode(VK_POLYGON_MODE_FILL);
    pipeline = builder.build(device, {});

    builder.clear();

    // shadow debug pipeline
    builder.setPipelineLayout(layout);
    builder.setShader(debugVert, VK_SHADER_STAGE_VERTEX_BIT);
    builder.setShader(debugFrag, VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.setDepthTest(false);
    builder.setCulling(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    builder.setPolygonMode(VK_POLYGON_MODE_FILL);
    builder.setMultisampleCount(g_graphics.getSampleCount());
    debugPipeline = builder.build(device, {});

    vkDestroyShaderModule(device, vertex, nullptr);
    vkDestroyShaderModule(device, debugVert, nullptr);
    vkDestroyShaderModule(device, debugFrag, nullptr);
}
void ShadowPipeline::destroy()
{
    VkDevice device = g_graphics.getDevice();

    vkDestroyPipelineLayout(device, layout, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipeline(device, debugPipeline, nullptr);
}

void ShadowPipeline::beginFrame(VkCommandBuffer cmd, const VkImageView shadowMap)
{
    const VkExtent2D extent = {shadowMapSize, shadowMapSize};

    // attachments
    VkRenderingAttachmentInfo depthAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    depthAttachment.clearValue.depthStencil = {0.0, 0};
    depthAttachment.imageView = shadowMap;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    float color[4] = {0.3, 0.3, 0.3, 0.3};
    vulkan::util::beginDebugLabel(cmd, "Shadow pass", color);

    vulkan::util::beginRendering(cmd, nullptr, 0, &depthAttachment, extent);

    vulkan::util::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::util::setScissor(cmd, extent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &g_graphics.getDescriptorManager().getSet(), 0, nullptr);
}

void ShadowPipeline::endFrame(VkCommandBuffer cmd, bool debug)
{
    vulkan::util::endRendering(cmd);
    vulkan::util::endDebugLabel(cmd);

    // shadow debug pass
    if (debug)
        debugDraw(cmd);
}

void ShadowPipeline::draw(VkCommandBuffer cmd, std::vector<MeshDraw> &meshDraws, mat4 lightMVP)
{
    vkCmdBindIndexBuffer(cmd, g_resourceManager.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    for (auto &command : meshDraws) {
        Mesh &mesh = g_resourceManager.getMesh(command.meshId);

        PushConstant pc = {
            .transform = lightMVP * command.transform,
        };

        vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
        vkCmdDrawIndexed(cmd, mesh.indexCount, 1, mesh.indexOffset, 0, 0);

        g_renderSettings.drawCount++;
    }
}

void ShadowPipeline::debugDraw(VkCommandBuffer cmd)
{
    Swapchain &swapchain = g_graphics.getSwapchain();
    const Image &colorImage = g_graphics.getColorImage();
    const VkImageView &swapchainImageView = swapchain.getImageView();
    const VkExtent2D extent = swapchain.getExtent();

    // attachments
    VkRenderingAttachmentInfo colorAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    colorAttachment.clearValue.color = {{0.0, 0.0, 0.0, 1.0}};
    colorAttachment.imageView = colorImage.view;
    colorAttachment.resolveImageView = swapchainImageView;
    colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    float color[4] = {0.3, 0.2, 0.3, 0.3};
    vulkan::util::beginDebugLabel(cmd, "Shadow debug pass", color);

    vulkan::util::beginRendering(cmd, &colorAttachment, 1, nullptr, extent);

    vulkan::util::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::util::setScissor(cmd, extent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, debugPipeline);
    vkCmdBindDescriptorSets(
        cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
        &g_graphics.getDescriptorManager().getSet(), 0, nullptr
    );

    // draw quad
    vkCmdDraw(cmd, 3, 1, 0, 0);

    g_renderSettings.drawCount++;

    vulkan::util::endRendering(cmd);
    vulkan::util::endDebugLabel(cmd);
}