#include <rebirth/graphics/renderer.h>

#include <rebirth/graphics/render_settings.h>
#include <rebirth/graphics/vulkan/util.h>

#include <backend/imgui_impl_sdl3.h>
#include <backend/imgui_impl_vulkan.h>
#include <imgui.h>

void Renderer::shadowPass(const VkCommandBuffer cmd)
{
    const Image &shadowMap = g_resourceManager.getImage(shadowMapId);
    const VkExtent2D shadowMapExtent = {SHADOW_MAP_SIZE, SHADOW_MAP_SIZE};

    // attachments
    VkRenderingAttachmentInfo depthAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    depthAttachment.clearValue.depthStencil = {0.0, 0};
    depthAttachment.imageView = shadowMap.view;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    float color[4] = {0.3, 0.3, 0.3, 0.3};
    vulkan::util::beginDebugLabel(cmd, "Shadow pass", color);

    vulkan::util::beginRendering(cmd, {}, &depthAttachment, shadowMapExtent);

    vulkan::util::setViewport(cmd, 0.0f, 0.0f, shadowMapExtent.width, shadowMapExtent.height);
    vulkan::util::setScissor(cmd, shadowMapExtent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines["shadow"]);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts["shadow"], 0, 1, &g_graphics.getDescriptorManager().getSet(), 0, nullptr);

    //
    // Draw
    //
    for (auto &light : g_resourceManager.lights) {
        for (uint32_t &opaqueDraw : opaqueDraws) {
            MeshDraw &meshDraw = meshDraws[opaqueDraw];
            if (meshDraw.meshId == MeshID::Invalid)
                continue;

            Mesh &mesh = g_resourceManager.getMesh(meshDraw.meshId);
            ShadowPassPC pc = {
                .transform = light.mvp * meshDraw.transform,
            };

            vkCmdPushConstants(cmd, pipelineLayouts["shadow"], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
            vkCmdDrawIndexed(cmd, mesh.indexCount, 1, mesh.indexOffset, 0, 0);

            g_renderSettings.drawCount++;
        }
    }

    // end
    vulkan::util::endRendering(cmd);
    vulkan::util::endDebugLabel(cmd);

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
    Swapchain &swapchain = g_graphics.getSwapchain();

    // TODO: make RenderInfo that would contain all information needed for a pipeline
    const VkExtent2D extent = swapchain.getExtent();
    const Image &colorImage = g_graphics.getColorImage();
    const Image &depthImage = g_graphics.getDepthImage();

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
    vulkan::util::beginDebugLabel(cmd, "Mesh pass", color);

    std::vector<VkRenderingAttachmentInfo> colorAttachments = {colorAttachment};
    vulkan::util::beginRendering(cmd, colorAttachments, &depthAttachment, extent);

    vulkan::util::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::util::setScissor(cmd, extent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g_renderSettings.drawWireframe ? pipelines["wireframe"] : pipelines["mesh"]);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts["mesh"], 0, 1, &g_graphics.getDescriptorManager().getSet(), 0, nullptr);

    //
    // Draw
    //
    for (uint32_t &opaqueDraw : opaqueDraws) {
        MeshDraw &meshDraw = meshDraws[opaqueDraw];
        if (meshDraw.meshId == MeshID::Invalid)
            continue;

        Mesh &mesh = g_resourceManager.getMesh(meshDraw.meshId);
        MeshPassPC pc = {
            .transform = meshDraw.transform,
            .materialId = mesh.materialId == MaterialID::Invalid ? -1 : int(mesh.materialId),
        };

        vkCmdPushConstants(cmd, pipelineLayouts["mesh"], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
        vkCmdDrawIndexed(cmd, mesh.indexCount, 1, mesh.indexOffset, 0, 0);

        g_renderSettings.drawCount++;
    }

    // end
    vulkan::util::endRendering(cmd);
    vulkan::util::endDebugLabel(cmd);
}

void Renderer::imGuiPass(const VkCommandBuffer cmd)
{
    Swapchain &swapchain = g_graphics.getSwapchain();

    const VkImageView &swapchainImageView = swapchain.getImageView();
    const Image &colorImage = g_graphics.getColorImage();
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
    vulkan::util::beginDebugLabel(cmd, "ImGui pass", color);

    std::vector<VkRenderingAttachmentInfo> colorAttachments = {colorAttachment};
    vulkan::util::beginRendering(cmd, colorAttachments, nullptr, extent);

    vulkan::util::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::util::setScissor(cmd, extent);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    //
    // Draw
    //
    if (g_renderSettings.drawImGui) {
        ImGui::ShowDemoWindow();

        //
        // Debug
        //
        ImGui::Begin("Debug");
        ImGui::Text("Frame time: %f ms", g_renderSettings.timestampDeltaMs);
        ImGui::Text("FPS: %d", int(1000.0f / g_renderSettings.timestampDeltaMs));
        ImGui::Text("Draw count: %d", g_renderSettings.drawCount);

        ImGui::Separator();

        ImGui::Checkbox("Enable wireframe", &g_renderSettings.drawWireframe);
        ImGui::Checkbox("Enable shadows", &g_renderSettings.drawShadows);
        ImGui::Checkbox("Enable skybox", &g_renderSettings.drawSkybox);
        ImGui::Checkbox("Enable imgui", &g_renderSettings.drawImGui);
        ImGui::End();

        //
        // Lights
        //
        ImGui::Begin("Lights");
        auto &lights = g_resourceManager.lights;
        for (size_t i = 0; i < lights.size(); i++) {
            if (lights[i].type == LightType::Point && ImGui::TreeNode(std::string("Light " + std::to_string(i)).c_str())) {
                ImGui::DragFloat3("position", &lights[i].position[0], 1.0f, -100.0f, 100.0f);

                ImGui::TreePop();
            }
        }
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    // end
    vulkan::util::endRendering(cmd);
    vulkan::util::endDebugLabel(cmd);
}

void Renderer::skyboxPass(const VkCommandBuffer cmd)
{
    vulkan::Swapchain &swapchain = g_graphics.getSwapchain();
    const Image &colorImage = g_graphics.getColorImage();
    const Image &depthImage = g_graphics.getDepthImage();
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
    vulkan::util::beginDebugLabel(cmd, "Skybox pass", color);

    std::vector<VkRenderingAttachmentInfo> colorAttachments = {colorAttachment};
    vulkan::util::beginRendering(cmd, colorAttachments, &depthAttachment, extent);

    vulkan::util::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::util::setScissor(cmd, extent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines["skybox"]);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts["skybox"], 0, 1, &g_graphics.getDescriptorManager().getSet(), 0, nullptr);

    //
    // Draw
    //
    if (cubeModelId != ModelID::Invalid) {
        Model &model = g_resourceManager.getModel(cubeModelId);
        for (MeshID meshId : model.meshes) {
            Mesh &mesh = g_resourceManager.getMesh(meshId);

            SkyboxPassPC pc = {
                .skyboxId = skyboxId != ImageID::Invalid ? int(skyboxId) : -1,
            };

            vkCmdPushConstants(cmd, pipelineLayouts["skybox"], VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
            vkCmdDrawIndexed(cmd, mesh.indexCount, 1, mesh.indexOffset, 0, 0);

            g_renderSettings.drawCount++;
        }
    }

    // end
    vulkan::util::endRendering(cmd);
    vulkan::util::endDebugLabel(cmd);
}

void Renderer::clearPass(const VkCommandBuffer cmd)
{
    const Image &colorImage = g_graphics.getColorImage();
    const Image &depthImage = g_graphics.getDepthImage();
    const Image &shadowMap = g_resourceManager.getImage(shadowMapId);
    // vulkan::Swapchain &swapchain = g_graphics.getSwapchain();
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