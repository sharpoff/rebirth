#include <rebirth/pipelines/shadow_pipeline.h>
#include <rebirth/util/logger.h>
#include <rebirth/vulkan/graphics.h>
#include <rebirth/vulkan/pipeline_builder.h>
#include <rebirth/vulkan/util.h>

namespace rebirth
{

void ShadowPipeline::initialize(Graphics &graphics)
{
    const VkDevice device = graphics.getDevice();
    DescriptorManager &descriptorManager = graphics.getDescriptorManager();

    // create pipeline layout
    VkPushConstantRange pushConstant = {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstant)};
    layout = graphics.createPipelineLayout(&descriptorManager.getSetLayout(), &pushConstant);

    const auto vertex = vulkan::loadShaderModule(device, "build/shaders/depth.vert.spv");

    const auto debugVert = vulkan::loadShaderModule(device, "build/shaders/quad.vert.spv");
    const auto debugFrag = vulkan::loadShaderModule(device, "build/shaders/shadow_debug.frag.spv");

    // shadow pipeline
    PipelineBuilder builder;
    builder.setPipelineLayout(layout);
    builder.setShader(vertex, VK_SHADER_STAGE_VERTEX_BIT);
    builder.setDepthTest(true);
    builder.setCulling(VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    builder.setPolygonMode(VK_POLYGON_MODE_FILL);
    pipeline = builder.build(device, 0);

    builder.clear();

    // shadow debug pipeline
    builder.setPipelineLayout(layout);
    builder.setShader(debugVert, VK_SHADER_STAGE_VERTEX_BIT);
    builder.setShader(debugFrag, VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.setDepthTest(false);
    builder.setCulling(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    builder.setPolygonMode(VK_POLYGON_MODE_FILL);
    builder.setMultisampleCount(graphics.getSampleCount());
    debugPipeline = builder.build(device, 1);

    vkDestroyShaderModule(device, vertex, nullptr);
    vkDestroyShaderModule(device, debugVert, nullptr);
    vkDestroyShaderModule(device, debugFrag, nullptr);
}
void ShadowPipeline::destroy(VkDevice device)
{
    vkDestroyPipelineLayout(device, layout, nullptr);
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipeline(device, debugPipeline, nullptr);
}

void ShadowPipeline::beginFrame(Graphics &graphics, VkCommandBuffer cmd)
{
    if (!shadowMap) {
        util::logWarn("Shadow map image is not set!");
        return;
    }

    const VkExtent2D extent = {shadowMapSize, shadowMapSize};

    // transfer to depth attachment
    VkImageMemoryBarrier depthBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = shadowMap->image,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}
    };

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &depthBarrier);

    // attachments
    VkRenderingAttachmentInfo depthAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    depthAttachment.clearValue.depthStencil = {0.0, 0};
    depthAttachment.imageView = shadowMap->view;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    float color[4] = {0.3, 0.3, 0.3, 0.3};
    vulkan::beginDebugLabel(cmd, "Shadow pass", color);

    vulkan::beginRendering(cmd, nullptr, 0, &depthAttachment, extent);

    vulkan::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::setScissor(cmd, extent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &graphics.getDescriptorManager().getSet(), 0, nullptr);
}

void ShadowPipeline::endFrame(Graphics &graphics, VkCommandBuffer cmd, bool debug)
{
    if (!shadowMap) {
        util::logWarn("Shadow map image is not set!");
        return;
    }

    vulkan::endRendering(cmd);

    // transfer to fragment shader read
    VkImageMemoryBarrier finalBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = shadowMap->image,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}
    };

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &finalBarrier);

    vulkan::endDebugLabel(cmd);

    // shadow debug pass
    if (debug)
        debugDraw(graphics, cmd);
}

void ShadowPipeline::draw(Graphics &graphics, ResourceManager &resourceManager, VkCommandBuffer cmd, std::vector<MeshDrawCommand> &drawCommands, mat4 lightMVP)
{
    if (!shadowMap) {
        util::logWarn("Shadow map image is not set!");
        return;
    }

    for (auto &command : drawCommands) {
        if (!command.castShadows)
            continue;

        GPUMesh &mesh = resourceManager.getMesh(command.meshIdx);

        vkCmdBindIndexBuffer(cmd, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        PushConstant pc = {
            .transform = lightMVP * command.transform,
            .vertexBuffer = mesh.vertexBuffer.address,
        };

        vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
        vkCmdDrawIndexed(cmd, mesh.indexCount, 1, 0, 0, 0);
    }
}

void ShadowPipeline::debugDraw(Graphics &graphics, VkCommandBuffer cmd)
{
    Swapchain &swapchain = graphics.getSwapchain();
    const Image &colorImage = graphics.getColorImage();
    const VkImage &swapchainImage = swapchain.getImage();
    const VkImageView &swapchainImageView = swapchain.getImageView();
    const VkExtent2D extent = swapchain.getExtent();

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

    float color[4] = {0.3, 0.2, 0.3, 0.3};
    vulkan::beginDebugLabel(cmd, "Shadow debug pass", color);

    vulkan::beginRendering(cmd, &colorAttachment, 1, nullptr, extent);

    vulkan::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::setScissor(cmd, extent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, debugPipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &graphics.getDescriptorManager().getSet(), 0, nullptr);

    // draw quad
    vkCmdDraw(cmd, 3, 1, 0, 0);

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

} // namespace rebirth