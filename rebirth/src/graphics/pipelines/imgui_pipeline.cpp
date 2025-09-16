#include <rebirth/graphics/pipelines/imgui_pipeline.h>
#include <rebirth/graphics/vulkan/graphics.h>
#include <rebirth/graphics/vulkan/pipeline_builder.h>
#include <rebirth/graphics/vulkan/util.h>

#include "backend/imgui_impl_sdl3.h"
#include "backend/imgui_impl_vulkan.h"
#include "imgui.h"

#include <rebirth/resource_manager.h>
#include <rebirth/graphics/render_settings.h>

using namespace vulkan;

void ImGuiPipeline::beginFrame(VkCommandBuffer cmd)
{
    Swapchain &swapchain = g_graphics.getSwapchain();

    const VkImageView &swapchainImageView = swapchain.getImageView();
    const Image &colorImage = g_graphics.getColorImage();
    const VkExtent2D extent = swapchain.getExtent();

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

    vulkan::util::beginRendering(cmd, &colorAttachment, 1, nullptr, extent);

    vulkan::util::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::util::setScissor(cmd, extent);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void ImGuiPipeline::endFrame(VkCommandBuffer cmd)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vulkan::util::endRendering(cmd);
    vulkan::util::endDebugLabel(cmd);
}

void ImGuiPipeline::draw()
{
    ImGui::ShowDemoWindow();

    {
        ImGui::Begin("Debug");

        ImGui::Text("Frame time: %f ms", g_renderSettings.timestampDeltaMs);
        ImGui::Text("FPS: %d", int(1000.0f / g_renderSettings.timestampDeltaMs));
        ImGui::Text("Draw count: %d", g_renderSettings.drawCount);

        ImGui::Separator();

        ImGui::Checkbox("Debug shadow map", &g_renderSettings.debugShadowMap);
        ImGui::Checkbox("Enable wireframe", &g_renderSettings.wireframe);
        ImGui::Checkbox("Enable shadows", &g_renderSettings.shadows);
        ImGui::Checkbox("Enable skybox", &g_renderSettings.skybox);

        {
            ImGui::Checkbox("Enable imgui", &g_renderSettings.imgui);

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("You can toggle it using 'H' key.");
            }
        }

        ImGui::End();
    }

    {
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
}