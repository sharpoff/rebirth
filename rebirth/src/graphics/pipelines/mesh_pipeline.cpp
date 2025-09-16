#include "rebirth/graphics/render_settings.h"
#include <rebirth/graphics/pipelines/mesh_pipeline.h>
#include <rebirth/graphics/vulkan/graphics.h>
#include <rebirth/graphics/vulkan/pipeline_builder.h>
#include <rebirth/graphics/vulkan/util.h>

#include <rebirth/math/frustum.h>
#include <rebirth/util/logger.h>

using namespace vulkan;

void MeshPipeline::initialize()
{
    const VkDevice device = g_graphics.getDevice();
    DescriptorManager &descriptorManager = g_graphics.getDescriptorManager();

    // if reloading destroy resources
    if (layout)
        vkDestroyPipelineLayout(device, layout, nullptr);
    if (meshPipeline)
        vkDestroyPipeline(device, meshPipeline, nullptr);
    if (wireframePipeline)
        vkDestroyPipeline(device, wireframePipeline, nullptr);

    // create pipeline layout
    VkPushConstantRange pushConstant = {
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        0,
        sizeof(PushConstant)};
    layout = g_graphics.createPipelineLayout(&descriptorManager.getSetLayout(), &pushConstant);

    const VkFormat colorFormat = g_graphics.getSwapchain().getSurfaceFormat().format;

    {
        const auto vertex = vulkan::util::loadShaderModule(device, "build/shaders/mesh.vert.spv");
        const auto fragment = vulkan::util::loadShaderModule(device, "build/shaders/mesh.frag.spv");

        // mesh pipeline
        PipelineBuilder builder;
        builder.setPipelineLayout(layout);
        builder.setShader(vertex, VK_SHADER_STAGE_VERTEX_BIT);
        builder.setShader(fragment, VK_SHADER_STAGE_FRAGMENT_BIT);
        builder.setDepthTest(true);
        builder.setCulling(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        builder.setPolygonMode(VK_POLYGON_MODE_FILL);
        builder.setMultisampleCount(g_graphics.getSampleCount());
        meshPipeline = builder.build(device, {colorFormat, colorFormat});

        vkDestroyShaderModule(device, vertex, nullptr);
        vkDestroyShaderModule(device, fragment, nullptr);
    }

    {
        const auto vertex = vulkan::util::loadShaderModule(device, "build/shaders/color.vert.spv");
        const auto fragment = vulkan::util::loadShaderModule(device, "build/shaders/color.frag.spv");

        // wireframe pipeline
        PipelineBuilder builder;
        builder.setPipelineLayout(layout);
        builder.setShader(vertex, VK_SHADER_STAGE_VERTEX_BIT);
        builder.setShader(fragment, VK_SHADER_STAGE_FRAGMENT_BIT);
        builder.setDepthTest(true);
        builder.setCulling(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        builder.setPolygonMode(VK_POLYGON_MODE_LINE);
        builder.setMultisampleCount(g_graphics.getSampleCount());
        wireframePipeline = builder.build(device, {colorFormat, colorFormat});

        vkDestroyShaderModule(device, vertex, nullptr);
        vkDestroyShaderModule(device, fragment, nullptr);
    }
}

void MeshPipeline::destroy()
{
    VkDevice device = g_graphics.getDevice();

    vkDestroyPipelineLayout(device, layout, nullptr);
    vkDestroyPipeline(device, meshPipeline, nullptr);
    vkDestroyPipeline(device, wireframePipeline, nullptr);
}

void MeshPipeline::beginFrame(VkCommandBuffer cmd, bool wireframe)
{
    Swapchain &swapchain = g_graphics.getSwapchain();

    // TODO: make RenderInfo that would contain all information needed for a pipeline
    const VkExtent2D extent = swapchain.getExtent();
    const Image &colorImage = g_graphics.getColorImage();
    const Image &colorImageOneSample = g_graphics.getColorImageOneSample();
    const Image &depthImage = g_graphics.getDepthImage();

    // attachments
    VkRenderingAttachmentInfo colorAttachment0 = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    colorAttachment0.clearValue.color = {{0.0, 0.0, 0.0, 1.0}};
    colorAttachment0.imageView = colorImage.view;
    colorAttachment0.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment0.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment0.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingAttachmentInfo colorAttachment1 = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    colorAttachment1.clearValue.color = {{0.0, 0.0, 0.0, 1.0}};
    colorAttachment1.imageView = colorImage.view;
    colorAttachment1.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment1.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment1.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment1.resolveImageView = colorImageOneSample.view;
    colorAttachment1.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment1.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;

    VkRenderingAttachmentInfo depthAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    depthAttachment.clearValue.depthStencil = {0.0, 0};
    depthAttachment.imageView = depthImage.view;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    float color[4] = {0.3, 0.3, 0.0, 0.3};
    vulkan::util::beginDebugLabel(cmd, "Mesh pass", color);

    std::array<VkRenderingAttachmentInfo, 2> attachments = {colorAttachment0, colorAttachment1};
    vulkan::util::beginRendering(cmd, attachments.data(), attachments.size(), &depthAttachment, extent);

    vulkan::util::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::util::setScissor(cmd, extent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, wireframe ? wireframePipeline : meshPipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &g_graphics.getDescriptorManager().getSet(), 0, nullptr);
}

void MeshPipeline::endFrame(VkCommandBuffer cmd)
{
    vulkan::util::endRendering(cmd);
    vulkan::util::endDebugLabel(cmd);
}

void MeshPipeline::draw(VkCommandBuffer cmd, Frustum &frustum, std::vector<MeshDraw> &meshDraws)
{
    vkCmdBindIndexBuffer(cmd, g_resourceManager.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    for (MeshDraw &meshDraw : meshDraws) {
        // if (!isInFrustum(frustum, command.boundingSphere, command.transform)) {
        //     continue;
        // }

        if (meshDraw.meshId == MeshID::Invalid)
            continue;

        Mesh &mesh = g_resourceManager.getMesh(meshDraw.meshId);

        PushConstant pc = {
            .transform = meshDraw.transform,
            .materialId = mesh.materialId == MaterialID::Invalid ? -1 : int(mesh.materialId),
        };

        vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
        vkCmdDrawIndexed(cmd, mesh.indexCount, 1, mesh.indexOffset, 0, 0);

        g_renderSettings.drawCount++;
    }
}