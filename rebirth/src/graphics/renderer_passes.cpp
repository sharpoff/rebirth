#include <rebirth/graphics/renderer.h>

#include <rebirth/graphics/vulkan/util.h>

#include <rebirth/util/logger.h>
#include <rebirth/core/cvar_system.h>

#include <backend/imgui_impl_sdl3.h>
#include <backend/imgui_impl_vulkan.h>
#include <imgui.h>

void Renderer::shadowPass(const VkCommandBuffer cmd)
{
    const Image &shadowMap = images[shadowMapIndex];
    const VkExtent2D shadowMapExtent = {SHADOW_MAP_SIZE, SHADOW_MAP_SIZE};

    // attachments
    VkRenderingAttachmentInfo depthAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    depthAttachment.clearValue.depthStencil = {0.0, 0};
    depthAttachment.imageView = shadowMap.view;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    float color[4] = {0.3, 0.3, 0.3, 0.3};
    vulkan::beginDebugLabel(cmd, "Shadow pass", color);

    vulkan::beginRendering(cmd, {}, &depthAttachment, shadowMapExtent);

    vulkan::setViewport(cmd, 0.0f, 0.0f, shadowMapExtent.width, shadowMapExtent.height);
    vulkan::setScissor(cmd, shadowMapExtent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines["shadow"]);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts["shadow"], 0, 1, &graphics.getDescriptorManager().getSet(), 0, nullptr);

    //
    // Draw
    //
    for (auto &light : lights) {
        for (uint32_t &opaqueDraw : opaqueDraws) {
            MeshDraw &meshDraw = meshDraws[opaqueDraw];
            Mesh &mesh = meshDraw.mesh;

            ShadowPassPC pc = {
                .transform = light.mvp * meshDraw.transform,
            };
            vkCmdPushConstants(cmd, pipelineLayouts["shadow"], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);

            for (Primitive &primitive : mesh.primitives) {
                if (primitive.indexCount > 0)
                    vkCmdDrawIndexed(cmd, primitive.indexCount, 1, primitive.indexOffset, primitive.vertexOffset, 0);
                else
                    vkCmdDraw(cmd, primitive.vertexCount, 1, primitive.vertexOffset, 0);
            }

            drawCount++;
        }
    }

    // end
    vulkan::endRendering(cmd);
    vulkan::endDebugLabel(cmd);

    // transfer shadowmap to fragment shader read
    VkImageMemoryBarrier shadowMapReadBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = shadowMap.image,
        .subresourceRange = depthSubresource};

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &shadowMapReadBarrier);
}

void Renderer::meshPass(const VkCommandBuffer cmd)
{
    Swapchain &swapchain = graphics.getSwapchain();

    // TODO: make RenderInfo that would contain all information needed for a pipeline
    const VkExtent2D extent = swapchain.getExtent();
    const Image &colorImage = graphics.getColorImage();
    const Image &depthImage = graphics.getDepthImage();

    // attachments
    VkRenderingAttachmentInfo colorAttachment;
    colorAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    colorAttachment.clearValue.color = {{0.0, 0.0, 0.0, 1.0}};
    colorAttachment.imageView = colorImage.view;
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

    eastl::vector<VkRenderingAttachmentInfo> colorAttachments = {colorAttachment};
    vulkan::beginRendering(cmd, colorAttachments, &depthAttachment, extent);

    vulkan::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::setScissor(cmd, extent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *CVarSystem::instance()->getCVarInt("render_wireframe") ? pipelines["wireframe"] : pipelines["mesh"]);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts["mesh"], 0, 1, &graphics.getDescriptorManager().getSet(), 0, nullptr);

    //
    // Draw
    //
    for (uint32_t &opaqueDraw : opaqueDraws) {
        MeshDraw &meshDraw = meshDraws[opaqueDraw];
        Mesh &mesh = meshDraw.mesh;

        for (Primitive &primitive : mesh.primitives) {
            MeshPassPC pc = {
                .transform = meshDraw.transform,
                .materialIndex = primitive.materialIndex,
            };

            vkCmdPushConstants(cmd, pipelineLayouts["mesh"], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);

            if (primitive.indexCount > 0)
                vkCmdDrawIndexed(cmd, primitive.indexCount, 1, primitive.indexOffset, primitive.vertexOffset, 0);
            else
                vkCmdDraw(cmd, primitive.vertexCount, 1, primitive.vertexOffset, 0);
        }

        drawCount++;
    }

    // end
    vulkan::endRendering(cmd);
    vulkan::endDebugLabel(cmd);
}

void Renderer::imGuiPass(const VkCommandBuffer cmd)
{
    Swapchain &swapchain = graphics.getSwapchain();

    const VkImageView &swapchainImageView = swapchain.getImageView();
    const Image &colorImage = graphics.getColorImage();
    const VkExtent2D extent = swapchain.getExtent();

    // attachments
    VkRenderingAttachmentInfo colorAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    colorAttachment.clearValue.color = {{0.0, 0.0, 0.0, 1.0}};
    colorAttachment.imageView = colorImage.view;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.resolveImageView = swapchainImageView;
    colorAttachment.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;

    float color[4] = {0.2, 0.2, 0.5, 0.3};
    vulkan::beginDebugLabel(cmd, "ImGui pass", color);

    eastl::vector<VkRenderingAttachmentInfo> colorAttachments = {colorAttachment};
    vulkan::beginRendering(cmd, colorAttachments, nullptr, extent);

    vulkan::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::setScissor(cmd, extent);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    //
    // Draw
    //
    if (*CVarSystem::instance()->getCVarInt("render_imgui")) {
        ImGui::ShowDemoWindow();

        //
        // Debug
        //
        ImGui::Begin("Debug");
        ImGui::Text("Frame time: %f ms", timestampDeltaMs);
        ImGui::Text("FPS: %d", int(1000.0f / timestampDeltaMs));
        ImGui::Text("Draw count: %d", drawCount);

        ImGui::Separator();

        ImGui::Checkbox("Enable wireframe", (bool*)CVarSystem::instance()->getCVarInt("render_wireframe"));
        ImGui::Checkbox("Enable shadows", (bool*)CVarSystem::instance()->getCVarInt("render_shadows"));
        ImGui::Checkbox("Enable skybox", (bool*)CVarSystem::instance()->getCVarInt("render_skybox"));
        ImGui::Checkbox("Enable imgui", (bool*)CVarSystem::instance()->getCVarInt("render_imgui"));
        ImGui::End();

        //
        // Lights
        //
        ImGui::Begin("Lights");
        for (size_t i = 0; i < lights.size(); i++) {
            if (lights[i].type == LightType::Point && ImGui::TreeNode(eastl::string("Light " + eastl::to_string(i)).c_str())) {
                ImGui::DragFloat3("position", &lights[i].position[0], 1.0f, -100.0f, 100.0f);

                ImGui::TreePop();
            }
        }
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    // end
    vulkan::endRendering(cmd);
    vulkan::endDebugLabel(cmd);
}

void Renderer::skyboxPass(const VkCommandBuffer cmd)
{
    vulkan::Swapchain &swapchain = graphics.getSwapchain();
    const Image &colorImage = graphics.getColorImage();
    const Image &depthImage = graphics.getDepthImage();
    const VkExtent2D extent = swapchain.getExtent();

    // attachments
    VkRenderingAttachmentInfo colorAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    colorAttachment.clearValue.color = {{0.0, 0.0, 0.0, 1.0}}; // not used when op is load
    colorAttachment.imageView = colorImage.view;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingAttachmentInfo depthAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    depthAttachment.clearValue.depthStencil = {0.0, 0}; // not used when op is load
    depthAttachment.imageView = depthImage.view;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    float color[4] = {0.3, 0.0, 3.0, 0.3};
    vulkan::beginDebugLabel(cmd, "Skybox pass", color);

    eastl::vector<VkRenderingAttachmentInfo> colorAttachments = {colorAttachment};
    vulkan::beginRendering(cmd, colorAttachments, &depthAttachment, extent);

    vulkan::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::setScissor(cmd, extent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines["skybox"]);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts["skybox"], 0, 1, &graphics.getDescriptorManager().getSet(), 0, nullptr);

    //
    // Draw
    //
    SkyboxPassPC pc = {
        .skyboxIndex = skyboxIndex,
    };

    vkCmdPushConstants(cmd, pipelineLayouts["skybox"], VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);

    if (cubePrimitive.indexCount > 0)
        vkCmdDrawIndexed(cmd, cubePrimitive.indexCount, 1, cubePrimitive.indexOffset, cubePrimitive.vertexOffset, 0);
    else
        vkCmdDraw(cmd, cubePrimitive.vertexCount, 1, cubePrimitive.vertexOffset, 0);

    drawCount++;

    // end
    vulkan::endRendering(cmd);
    vulkan::endDebugLabel(cmd);
}

void Renderer::clearPass(const VkCommandBuffer cmd)
{
    const Image &colorImage = graphics.getColorImage();
    const Image &depthImage = graphics.getDepthImage();
    const Image &shadowMap = images[shadowMapIndex];
    // vulkan::Swapchain &swapchain = graphics.getSwapchain();
    // const VkExtent2D extent = swapchain.getExtent();

    // transfer multisampled image to transfer dst
    VkImageMemoryBarrier transferImageBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = colorImage.image,
        .subresourceRange = colorSubresource};

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &transferImageBarrier);

    // transfer depth image to transfer
    VkImageMemoryBarrier depthTransferBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = depthImage.image,
        .subresourceRange = depthSubresource};

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &depthTransferBarrier);

    // transfer shadowmap image to transfer
    depthTransferBarrier.image = shadowMap.image;
    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &depthTransferBarrier);

    VkClearDepthStencilValue clearDepthVal = {0.0, 0};
    VkClearColorValue clearColorVal = {{0.0, 0.0, 0.0, 1.0}};

    VkImageSubresourceRange depthRange{};
    depthRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthRange.baseMipLevel = 0;
    depthRange.levelCount = 1;
    depthRange.baseArrayLayer = 0;
    depthRange.layerCount = 1;

    VkImageSubresourceRange multisampledColorRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = colorImage.mipLevels,
        .baseArrayLayer = 0,
        .layerCount = 1};

    //
    // Clear images
    //
    vkCmdClearDepthStencilImage(cmd, depthImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearDepthVal, 1, &depthRange);
    vkCmdClearDepthStencilImage(cmd, shadowMap.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearDepthVal, 1, &depthRange);
    vkCmdClearColorImage(cmd, colorImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColorVal, 1, &multisampledColorRange);

    // transfer multisampled image to color output
    VkImageMemoryBarrier multisampleBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = colorImage.image,
        .subresourceRange = colorSubresource};

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &multisampleBarrier);

    // transfer depth image to depth attachment
    VkImageMemoryBarrier depthBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = depthImage.image,
        .subresourceRange = depthSubresource};

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &depthBarrier);

    // transfer shadowmap image to depth attachment
    depthBarrier.image = shadowMap.image;
    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &depthBarrier);
}