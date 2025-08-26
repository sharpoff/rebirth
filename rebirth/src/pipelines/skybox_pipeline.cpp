#include <rebirth/pipelines/skybox_pipeline.h>
#include <rebirth/vulkan/graphics.h>
#include <rebirth/vulkan/pipeline_builder.h>
#include <rebirth/vulkan/util.h>

namespace rebirth
{
void SkyboxPipeline::initialize(Graphics &graphics)
{
    const VkDevice device = graphics.getDevice();
    DescriptorManager &descriptorManager = graphics.getDescriptorManager();

    // create pipeline layout
    VkPushConstantRange pushConstant = {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant)};
    layout = graphics.createPipelineLayout(&descriptorManager.getSetLayout(), &pushConstant);

    const auto vertex = vulkan::loadShaderModule(device, "build/shaders/skybox.vert.spv");
    const auto fragment = vulkan::loadShaderModule(device, "build/shaders/skybox.frag.spv");

    // skybox pipeline
    PipelineBuilder builder;
    builder.setPipelineLayout(layout);
    builder.setShader(vertex, VK_SHADER_STAGE_VERTEX_BIT);
    builder.setShader(fragment, VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.setCulling(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    builder.setDepthTest(false);
    builder.setMultisampleCount(graphics.getSampleCount());
    pipeline = builder.build(device, 1);

    vkDestroyShaderModule(device, vertex, nullptr);
    vkDestroyShaderModule(device, fragment, nullptr);

    // create buffers
    graphics.createBuffer(&vertexBuffer, vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    graphics.createBuffer(&indexBuffer, indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    graphics.uploadBuffer(vertexBuffer, vertices.data(), vertices.size() * sizeof(Vertex));
    graphics.uploadBuffer(indexBuffer, indices.data(), indices.size() * sizeof(uint32_t));
}

void SkyboxPipeline::destroy(Graphics &graphics)
{
    const VkDevice device = graphics.getDevice();

    vkDestroyPipelineLayout(device, layout, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);

    graphics.destroyBuffer(&vertexBuffer);
    graphics.destroyBuffer(&indexBuffer);
}

void SkyboxPipeline::beginFrame(Graphics &graphics, VkCommandBuffer cmd)
{
    Swapchain &swapchain = graphics.getSwapchain();

    const VkImage &swapchainImage = swapchain.getImage();
    const VkImageView &swapchainImageView = swapchain.getImageView();
    const VkExtent2D extent = swapchain.getExtent();
    const Image &colorImage = graphics.getColorImage();

    // transfer multisampled image to color attachment
    VkImageMemoryBarrier colorBarrier0 = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = colorImage.image,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}
    };

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &colorBarrier0);

    // transfer swapchain image to color attachment
    VkImageMemoryBarrier colorBarrier1 = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapchainImage,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}
    };

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &colorBarrier1);

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

    float color[4] = {0.3, 0.0, 3.0, 0.3};
    vulkan::beginDebugLabel(cmd, "Skybox pass", color);

    vulkan::beginRendering(cmd, &colorAttachment, 1, nullptr, extent);

    vulkan::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::setScissor(cmd, extent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &graphics.getDescriptorManager().getSet(), 0, nullptr);
}

void SkyboxPipeline::endFrame(Graphics &graphics, VkCommandBuffer cmd)
{
    const VkImage &swapchainImage = graphics.getSwapchain().getImage();

    vulkan::endRendering(cmd);

    // transfer swapchain image to present
    VkImageMemoryBarrier presentBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapchainImage,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}
    };

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &presentBarrier);
    vulkan::endDebugLabel(cmd);
}

void SkyboxPipeline::drawSkybox(Graphics &graphics, VkCommandBuffer cmd, int skyboxIndex)
{
    vkCmdBindIndexBuffer(cmd, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    PushConstant pc = {
        .vertexBuffer = vertexBuffer.address,
        .skyboxIndex = skyboxIndex,
    };

    vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
    vkCmdDrawIndexed(cmd, indices.size(), 1, 0, 0, 0);
}

} // namespace rebirth