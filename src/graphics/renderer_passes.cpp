#include "core/mesh_draw.h"
#include "graphics/renderer.h"

#include "core/engine_stats.h"
#include "core/resource_manager.h"
#include "graphics/vulkan/util.h"

#include "backend/imgui_impl_sdl3.h"
#include "backend/imgui_impl_vulkan.h"
#include "imgui.h"

void Renderer::shadowPass(const VkCommandBuffer cmd)
{
    const Image *shadowMap = ResourceManager::get()->getImageByName("shadow_map");
    if (!shadowMap)
        return;

    const VkExtent2D shadowMapExtent = {SHADOW_MAP_SIZE, SHADOW_MAP_SIZE};

    // attachments
    VkRenderingAttachmentInfo depthAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    depthAttachment.clearValue.depthStencil = {0.0, 0};
    depthAttachment.imageView = shadowMap->view;
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
    for (auto &light : ResourceManager::get()->getLights()) {
        for (uint32_t &draw : meshDrawIndices) {
            MeshDraw &meshDraw = meshDraws[draw];
            if ((meshDraw.drawMask & DrawMask::Shadow) != DrawMask::Shadow)
                continue;

            Mesh *mesh = ResourceManager::get()->getMeshByIndex(meshDraw.meshId);
            if (!mesh)
                continue;

            ShadowPassPC pc = {
                .transform = light.mvp * meshDraw.transform * mesh->transform,
            };
            vkCmdPushConstants(cmd, pipelineLayouts["shadow"], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);

            for (Primitive &primitive : mesh->primitives) {
                if (primitive.indexCount > 0)
                    vkCmdDrawIndexed(cmd, primitive.indexCount, 1, primitive.indexOffset, primitive.vertexOffset, 0);
                else
                    vkCmdDraw(cmd, primitive.vertexCount, 1, primitive.vertexOffset, 0);
            }

            engineStats->drawCount++;
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
        .image = shadowMap->image,
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
    const Swapchain &swapchain = graphics.getSwapchain();

    // TODO: make RenderInfo that would contain all information needed for a pipeline
    const VkExtent2D extent = swapchain.getExtent();
    const Image     &colorImage = graphics.getColorImage();
    const Image     &depthImage = graphics.getDepthImage();

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

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts["mesh"], 0, 1, &graphics.getDescriptorManager().getSet(), 0, nullptr);

    //
    // Draw
    //
    VkPipeline *currentPipeline = &pipelines["mesh_opaque"];
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *currentPipeline);

    for (uint32_t &draw : meshDrawIndices) {
        MeshDraw &meshDraw = meshDraws[draw];
        Mesh *mesh = ResourceManager::get()->getMeshByIndex(meshDraw.meshId);
        if (!mesh)
            continue;

        // HACK: don't render overlay here
        if ((meshDraw.drawMask & DrawMask::Overlay) == DrawMask::Overlay)
            continue;

        // set pipeline
        // XXX: this code is ugly as hell
        if ((meshDraw.drawMask & DrawMask::Opaque) == DrawMask::Opaque && currentPipeline != &pipelines["mesh_opaque"]) {
            currentPipeline = &pipelines["mesh_opaque"];
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *currentPipeline);
        } else if ((meshDraw.drawMask & DrawMask::Wireframe) == DrawMask::Wireframe && currentPipeline != &pipelines["wireframe"]) {
            currentPipeline = &pipelines["wireframe"];
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *currentPipeline);
        } else if ((meshDraw.drawMask & DrawMask::Transparent) == DrawMask::Transparent && currentPipeline != &pipelines["mesh_transparent"]) {
            currentPipeline = &pipelines["mesh_transparent"];
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *currentPipeline);
        }

        for (Primitive &primitive : mesh->primitives) {
            MeshPassPC pc = {
                .transform = meshDraw.transform * mesh->transform,
                .materialIndex = meshDraw.overrideMaterialId > -1 ? meshDraw.overrideMaterialId : primitive.materialIndex,
                .drawMask = meshDraw.drawMask,
            };

            vkCmdPushConstants(cmd, pipelineLayouts["mesh"], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);

            if (primitive.indexCount > 0)
                vkCmdDrawIndexed(cmd, primitive.indexCount, 1, primitive.indexOffset, primitive.vertexOffset, 0);
            else
                vkCmdDraw(cmd, primitive.vertexCount, 1, primitive.vertexOffset, 0);
        }

        engineStats->drawCount++;
    }

    // end
    vulkan::endRendering(cmd);
    vulkan::endDebugLabel(cmd);
}

void Renderer::overlayPass(const VkCommandBuffer cmd)
{
    const Swapchain &swapchain = graphics.getSwapchain();

    // TODO: make RenderInfo that would contain all information needed for a pipeline
    const VkExtent2D extent = swapchain.getExtent();
    const Image     &colorImage = graphics.getColorImage();
    const Image     &depthImage = graphics.getDepthImage();

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

    float color[4] = {0.3, 0.0, 0.1, 1.0};
    vulkan::beginDebugLabel(cmd, "Overlay pass", color);

    eastl::vector<VkRenderingAttachmentInfo> colorAttachments = {colorAttachment};
    vulkan::beginRendering(cmd, colorAttachments, &depthAttachment, extent);

    vulkan::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::setScissor(cmd, extent);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts["mesh"], 0, 1, &graphics.getDescriptorManager().getSet(), 0, nullptr);

    //
    // Draw
    //
    VkPipeline *currentPipeline = &pipelines["mesh_opaque"];
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *currentPipeline);

    for (uint32_t &draw : meshDrawIndices) {
        MeshDraw &meshDraw = meshDraws[draw];
        Mesh *mesh = ResourceManager::get()->getMeshByIndex(meshDraw.meshId);
        if (!mesh)
            continue;

        // HACK: render ONLY overlay here
        if ((meshDraw.drawMask & DrawMask::Overlay) != DrawMask::Overlay)
            continue;

        // set pipeline
        // XXX: this code is ugly as hell
        if ((meshDraw.drawMask & DrawMask::Opaque) == DrawMask::Opaque && currentPipeline != &pipelines["mesh_opaque"]) {
            currentPipeline = &pipelines["mesh_opaque"];
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *currentPipeline);
        } else if ((meshDraw.drawMask & DrawMask::Wireframe) == DrawMask::Wireframe && currentPipeline != &pipelines["wireframe"]) {
            currentPipeline = &pipelines["wireframe"];
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *currentPipeline);
        } else if ((meshDraw.drawMask & DrawMask::Transparent) == DrawMask::Transparent && currentPipeline != &pipelines["mesh_transparent"]) {
            currentPipeline = &pipelines["mesh_transparent"];
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *currentPipeline);
        }

        for (Primitive &primitive : mesh->primitives) {
            MeshPassPC pc = {
                .transform = meshDraw.transform * mesh->transform,
                .materialIndex = meshDraw.overrideMaterialId > -1 ? meshDraw.overrideMaterialId : primitive.materialIndex,
                .drawMask = meshDraw.drawMask,
            };

            vkCmdPushConstants(cmd, pipelineLayouts["mesh"], VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);

            if (primitive.indexCount > 0)
                vkCmdDrawIndexed(cmd, primitive.indexCount, 1, primitive.indexOffset, primitive.vertexOffset, 0);
            else
                vkCmdDraw(cmd, primitive.vertexCount, 1, primitive.vertexOffset, 0);
        }

        engineStats->drawCount++;
    }

    // end
    vulkan::endRendering(cmd);
    vulkan::endDebugLabel(cmd);
}

void Renderer::imGuiPass(const VkCommandBuffer cmd, Editor *editor)
{
    if (!editor) return;

    Swapchain &swapchain = graphics.getSwapchain();

    const VkImageView &swapchainImageView = swapchain.getImageView();
    const Image       &colorImage = graphics.getColorImage();
    const VkExtent2D   extent = swapchain.getExtent();

    // FIXME: syncronizaiton issues somewhere here
    // transfer multisampled image to color write
    // VkImageMemoryBarrier barrier0 = {
    //     .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    //     .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    //     .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    //     .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    //     .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    //     .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    //     .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    //     .image = colorImage.image,
    //     .subresourceRange = colorSubresource};

    // vkCmdPipelineBarrier(
    //     cmd,
    //     VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    //     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //     0,
    //     0,
    //     nullptr,
    //     0,
    //     nullptr,
    //     1,
    //     &barrier0);

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

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGuiID        dockspaceId = ImGui::GetID("Dockspace");

    ImGui::DockSpaceOverViewport(dockspaceId, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

    //
    // Draw
    //
    editor->drawEditor();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    // end
    vulkan::endRendering(cmd);
    vulkan::endDebugLabel(cmd);
}

void Renderer::skyboxPass(const VkCommandBuffer cmd)
{
    Mesh *cubeMesh = ResourceManager::get()->getMeshByName("Cube");
    if (!cubeMesh)
        return;

    vulkan::Swapchain &swapchain = graphics.getSwapchain();
    const Image       &colorImage = graphics.getColorImage();
    const Image       &depthImage = graphics.getDepthImage();
    const VkExtent2D   extent = swapchain.getExtent();

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
        .skyboxIndex = ResourceManager::get()->getImageIndexByName("skybox"),
    };

    vkCmdPushConstants(cmd, pipelineLayouts["skybox"], VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);

    for (Primitive &primitive : cubeMesh->primitives) {
        if (primitive.indexCount > 0)
            vkCmdDrawIndexed(cmd, primitive.indexCount, 1, primitive.indexOffset, primitive.vertexOffset, 0);
        else
            vkCmdDraw(cmd, primitive.vertexCount, 1, primitive.vertexOffset, 0);
    }

    engineStats->drawCount++;

    // end
    vulkan::endRendering(cmd);
    vulkan::endDebugLabel(cmd);
}

void Renderer::clearPass(const VkCommandBuffer cmd)
{
    const Image &colorImage = graphics.getColorImage();
    const Image &depthImage = graphics.getDepthImage();
    const Image *shadowMap = ResourceManager::get()->getImageByName("shadow_map");

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
    if (shadowMap) {
        depthTransferBarrier.image = shadowMap->image;
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
    }

    VkClearDepthStencilValue clearDepthVal = {0.0, 0};
    VkClearColorValue        clearColorVal = {{0.0, 0.0, 0.0, 1.0}};

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

    float color[4] = {0.2, 0.3, 0.5, 0.3};
    vulkan::beginDebugLabel(cmd, "Clear pass", color);

    //
    // Clear images
    //
    vkCmdClearColorImage(cmd, colorImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColorVal, 1, &multisampledColorRange);
    vkCmdClearDepthStencilImage(cmd, depthImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearDepthVal, 1, &depthRange);
    if (shadowMap)
        vkCmdClearDepthStencilImage(cmd, shadowMap->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearDepthVal, 1, &depthRange);

    // transfer multisampled image to color output
    VkImageMemoryBarrier multisampleBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
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
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
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
    if (shadowMap) {
        depthBarrier.image = shadowMap->image;
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

    vulkan::endDebugLabel(cmd);
}