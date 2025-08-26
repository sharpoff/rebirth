#include <rebirth/pipelines/mesh_pipeline.h>
#include <rebirth/vulkan/graphics.h>
#include <rebirth/vulkan/pipeline_builder.h>
#include <rebirth/vulkan/util.h>

namespace rebirth
{

void MeshPipeline::initialize(Graphics &graphics)
{
    const VkDevice device = graphics.getDevice();
    DescriptorManager &descriptorManager = graphics.getDescriptorManager();

    // create pipeline layout
    VkPushConstantRange pushConstant = {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant)};
    layout = graphics.createPipelineLayout(&descriptorManager.getSetLayout(), &pushConstant);

    const auto vertex = vulkan::loadShaderModule(device, "build/shaders/mesh.vert.spv");
    const auto fragment = vulkan::loadShaderModule(device, "build/shaders/mesh.frag.spv");

    // mesh pipeline
    PipelineBuilder builder;
    builder.setPipelineLayout(layout);
    builder.setShader(vertex, VK_SHADER_STAGE_VERTEX_BIT);
    builder.setShader(fragment, VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.setDepthTest(true);
    builder.setCulling(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    builder.setPolygonMode(VK_POLYGON_MODE_FILL);
    builder.setMultisampleCount(graphics.getSampleCount());
    pipeline = builder.build(device, 1);

    vkDestroyShaderModule(device, vertex, nullptr);
    vkDestroyShaderModule(device, fragment, nullptr);
}

void MeshPipeline::destroy(VkDevice device)
{
    vkDestroyPipelineLayout(device, layout, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
}

void MeshPipeline::beginFrame(Graphics &graphics, VkCommandBuffer cmd)
{
    Swapchain &swapchain = graphics.getSwapchain();

    const VkImage &swapchainImage = swapchain.getImage();
    const VkImageView &swapchainImageView = swapchain.getImageView();
    const VkExtent2D extent = swapchain.getExtent();
    const Image &colorImage = graphics.getColorImage();
    const Image &depthImage = graphics.getDepthImage();

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

    // transfer to depth attachment
    VkImageMemoryBarrier depthBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = depthImage.image,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}
    };

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &depthBarrier);

    // attachments
    VkRenderingAttachmentInfo colorAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    colorAttachment.clearValue.color = {{0.0, 0.0, 0.0, 1.0}};
    colorAttachment.imageView = colorImage.view;
    colorAttachment.resolveImageView = swapchainImageView;
    colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingAttachmentInfo depthAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    depthAttachment.clearValue.depthStencil = {0.0, 0};
    depthAttachment.imageView = depthImage.view;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    float color[4] = {0.3, 0.3, 0.0, 0.3};
    vulkan::beginDebugLabel(cmd, "Mesh pass", color);

    vulkan::beginRendering(cmd, &colorAttachment, 1, &depthAttachment, extent);

    vulkan::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::setScissor(cmd, extent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &graphics.getDescriptorManager().getSet(), 0, nullptr);
}

void MeshPipeline::endFrame(Graphics &graphics, VkCommandBuffer cmd)
{
    Swapchain &swapchain = graphics.getSwapchain();
    const VkImage &swapchainImage = swapchain.getImage();

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

void MeshPipeline::draw(Graphics &graphics, ResourceManager &resourceManager, VkCommandBuffer cmd, std::vector<MeshDrawCommand> &drawCommands)
{
    for (auto &command : drawCommands) {
        GPUMesh &mesh = resourceManager.getMesh(command.meshIdx);

        vkCmdBindIndexBuffer(cmd, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        PushConstant pc = {
            .transform = command.transform,
            .vertexBuffer = mesh.vertexBuffer.address,
            .materialIdx = mesh.materialIdx,
        };

        vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
        vkCmdDrawIndexed(cmd, mesh.indexCount, 1, 0, 0, 0);
    }
}

} // namespace rebirth