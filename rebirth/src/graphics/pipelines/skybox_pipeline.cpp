#include <rebirth/graphics/pipelines/skybox_pipeline.h>
#include <rebirth/graphics/primitives.h>
#include <rebirth/graphics/vulkan/graphics.h>
#include <rebirth/graphics/vulkan/pipeline_builder.h>
#include <rebirth/graphics/vulkan/util.h>

#include "rebirth/graphics/render_settings.h"
#include <rebirth/resource_manager.h>

using namespace vulkan;

void SkyboxPipeline::initialize(ModelID cubeModelId)
{
    const VkDevice device = g_graphics.getDevice();
    DescriptorManager &descriptorManager = g_graphics.getDescriptorManager();

    // if reloading destroy resources
    if (layout)
        vkDestroyPipelineLayout(device, layout, nullptr);
    if (pipeline)
        vkDestroyPipeline(device, pipeline, nullptr);

    // create pipeline layout
    VkPushConstantRange pushConstant = {VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant)};
    layout = g_graphics.createPipelineLayout(&descriptorManager.getSetLayout(), &pushConstant);

    const auto vertex = vulkan::util::loadShaderModule(device, "build/shaders/skybox.vert.spv");
    const auto fragment = vulkan::util::loadShaderModule(device, "build/shaders/skybox.frag.spv");

    const VkFormat colorFormat = g_graphics.getSwapchain().getSurfaceFormat().format;

    // skybox pipeline
    PipelineBuilder builder;
    builder.setPipelineLayout(layout);
    builder.setShader(vertex, VK_SHADER_STAGE_VERTEX_BIT);
    builder.setShader(fragment, VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.setCulling(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    builder.setDepthTest(false);
    builder.setMultisampleCount(g_graphics.getSampleCount());
    pipeline = builder.build(device, {colorFormat});

    vkDestroyShaderModule(device, vertex, nullptr);
    vkDestroyShaderModule(device, fragment, nullptr);

    this->cubeModelId = cubeModelId;
}

void SkyboxPipeline::destroy()
{
    VkDevice device = g_graphics.getDevice();

    vkDestroyPipelineLayout(device, layout, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
}

void SkyboxPipeline::beginFrame(VkCommandBuffer cmd)
{
    vulkan::Swapchain &swapchain = g_graphics.getSwapchain();
    const Image &colorImage = g_graphics.getColorImage();
    const VkExtent2D extent = swapchain.getExtent();

    // attachments
    VkRenderingAttachmentInfo colorAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    colorAttachment.clearValue.color = {{0.0, 0.0, 0.0, 1.0}};
    colorAttachment.imageView = colorImage.view;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    float color[4] = {0.3, 0.0, 3.0, 0.3};
    vulkan::util::beginDebugLabel(cmd, "Skybox pass", color);

    vulkan::util::beginRendering(cmd, &colorAttachment, 1, nullptr, extent);

    vulkan::util::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::util::setScissor(cmd, extent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &g_graphics.getDescriptorManager().getSet(), 0, nullptr);
}

void SkyboxPipeline::endFrame(VkCommandBuffer cmd)
{
    vulkan::util::endRendering(cmd);
    vulkan::util::endDebugLabel(cmd);
}

void SkyboxPipeline::drawSkybox(VkCommandBuffer cmd, ImageID skyboxId)
{
    vkCmdBindIndexBuffer(cmd, g_resourceManager.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    if (cubeModelId != ModelID::Invalid) {
        Model &model = g_resourceManager.getModel(cubeModelId);
        for (MeshID meshId : model.meshes) {
            Mesh &mesh = g_resourceManager.getMesh(meshId);

            PushConstant pc = {
                .skyboxId = skyboxId != ImageID::Invalid ? int(skyboxId) : -1,
            };

            vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
            vkCmdDrawIndexed(cmd, mesh.indexCount, 1, mesh.indexOffset, 0, 0);

            g_renderSettings.drawCount++;
        }
    }
}