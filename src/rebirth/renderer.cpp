#include <rebirth/gltf.h>
#include <rebirth/renderer.h>
#include <rebirth/vulkan/descriptor_writer.h>
#include <rebirth/vulkan/pipeline_builder.h>
#include <rebirth/vulkan/swapchain.h>
#include <rebirth/vulkan/util.h>

#include "backend/imgui_impl_sdl3.h"
#include "backend/imgui_impl_vulkan.h"
#include "imgui.h"

namespace rebirth
{

Renderer::Renderer(SDL_Window *window) : window(window)
{
    assert(window);

    graphics = new vulkan::Graphics(window);
    const VkDevice device = graphics->getDevice();

    // load scenes
    std::vector<std::filesystem::path> texturePaths;
    if (auto fox = gltf::loadScene(*graphics, texturePaths, materials, "assets/models/Fox.gltf"); fox.has_value()) {
        scenes["fox"] = fox.value();
        scenes["fox"].transform.scale(vec3(0.1));
    }

    if (auto cube = gltf::loadScene(*graphics, texturePaths, materials, "assets/models/cube.gltf"); cube.has_value()) {
        scenes["cube"] = cube.value();
    }

    // load textures from scenes
    for (auto path : texturePaths) {
        vulkan::Image texture;
        graphics->createImageFromFile(&texture, path);
        textures.push_back(texture);
    }

    // load shaders
    for (auto file : std::filesystem::recursive_directory_iterator("build/shaders")) {
        if (!file.is_regular_file())
            continue;
        shaders[file.path().filename()] = vulkan::loadShaderModule(device, file.path());
    }

    // load lights
    lights.push_back(Light{.position = vec3(2, 40, 0)});

    createResources();
}

Renderer::~Renderer()
{
    const VkDevice device = graphics->getDevice();
    vkDeviceWaitIdle(device);

    for (auto &[_, scene] : scenes) {
        for (auto &mesh : scene.meshes) {
            graphics->destroyBuffer(&mesh.indexBuffer);
            graphics->destroyBuffer(&mesh.vertexBuffer);
        }
    }

    for (auto &[_, shader] : shaders) {
        vkDestroyShaderModule(device, shader, nullptr);
    }

    for (vulkan::Image tex : textures) {
        graphics->destroyImage(&tex);
    }

    graphics->destroyImage(&colorImage);
    graphics->destroyImage(&depthImage);
    graphics->destroyImage(&shadowMap);
    graphics->destroyImage(&skybox);

    graphics->destroyBuffer(&uboBuffer);
    graphics->destroyBuffer(&materialsBuffer);
    graphics->destroyBuffer(&lightsBuffer);

    vkDestroyDescriptorPool(device, pool, nullptr);
    vkDestroyDescriptorSetLayout(device, setLayout, nullptr);

    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyPipeline(device, pipelines.scene, nullptr);
    vkDestroyPipeline(device, pipelines.shadow, nullptr);
    vkDestroyPipeline(device, pipelines.skybox, nullptr);

    delete graphics;
}

void Renderer::render()
{
    updateDynamicData();

    const VkCommandBuffer cmd = graphics->beginCommandBuffer();

    // resizing or failed to acquire swapchain image
    if (cmd == VK_NULL_HANDLE) {
        graphics->destroyImage(&colorImage);
        graphics->destroyImage(&depthImage);

        createColorImage();
        createDepthImage();
        return;
    }

    // Record render passes
    recordSkyboxPass(cmd);
    recordShadowPass(cmd);
    recordMeshPass(cmd);
    recordImGuiPass(cmd);

    graphics->submitCommandBuffer(cmd);
}

void Renderer::requestResize() const { graphics->requestResize(); }

void Renderer::recordShadowPass(VkCommandBuffer cmd)
{
    const VkExtent2D extent = {shadowMapSize, shadowMapSize};

    VkRenderingAttachmentInfo depthAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    depthAttachment.clearValue.depthStencil = {0.0, 0};
    depthAttachment.imageView = shadowMap.view;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // transfer to depth attachment
    VkImageMemoryBarrier depthBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = shadowMap.image,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}
    };

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1, &depthBarrier);

    float color[4] = {0.0, 0.1, 0.1, 1.0};
    vulkan::beginDebugLabel(cmd, "Shadow pass", color);

    vulkan::beginRendering(cmd, nullptr, 0, &depthAttachment, extent);

    vulkan::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::setScissor(cmd, extent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.shadow);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &set, 0, nullptr);

    for (auto &light : lights) {
        mat4 lightMVP = light.mvp;

        // draw
        auto &scene = scenes["fox"];
        for (auto &mesh : scene.meshes) {
            vkCmdBindIndexBuffer(cmd, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

            GPUMeshPushConstant pc;
            pc.address = mesh.vertexBuffer.address;
            pc.worldMatrix = lightMVP * mesh.matrix * scene.transform.getModelMatrix();
            pc.materialIdx = mesh.materialIdx;

            vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
            vkCmdDrawIndexed(cmd, mesh.indexCount, 1, 0, 0, 0);
        }
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
        .image = shadowMap.image,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}
    };

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &finalBarrier);

    vulkan::endDebugLabel(cmd);
}

void Renderer::recordMeshPass(VkCommandBuffer cmd)
{
    vulkan::Swapchain &swapchain = graphics->getSwapchain();

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

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.scene);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &set, 0, nullptr);

    //
    // Draw
    //
    auto &scene = scenes["fox"];
    for (auto &mesh : scene.meshes) {
        vkCmdBindIndexBuffer(cmd, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        GPUMeshPushConstant pc;
        pc.address = mesh.vertexBuffer.address;
        pc.worldMatrix = mesh.matrix * scene.transform.getModelMatrix();
        pc.materialIdx = mesh.materialIdx;

        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
        vkCmdDrawIndexed(cmd, mesh.indexCount, 1, 0, 0, 0);
    }

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

void Renderer::recordImGuiPass(VkCommandBuffer cmd)
{
    vulkan::Swapchain &swapchain = graphics->getSwapchain();

    const VkImage &swapchainImage = swapchain.getImage();
    const VkImageView &swapchainImageView = swapchain.getImageView();
    const VkExtent2D extent = swapchain.getExtent();

    VkRenderingAttachmentInfo colorAttachment = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    colorAttachment.clearValue.color = {{0.0, 0.0, 0.0, 1.0}};
    colorAttachment.imageView = swapchainImageView;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // transfer to color attachment
    VkImageMemoryBarrier colorBarrier = {
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

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &colorBarrier);

    float color[4] = {0.2, 0.2, 0.5, 0.3};
    vulkan::beginDebugLabel(cmd, "ImGui pass", color);

    vulkan::beginRendering(cmd, &colorAttachment, 1, nullptr, extent);

    vulkan::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::setScissor(cmd, extent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.scene);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &set, 0, nullptr);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    //
    // Draw ImGui
    //
    ImGui::ShowDemoWindow();

    {
        auto camPos = camera->getPosition();

        ImGui::Begin("Debug");
        ImGui::Text("Camera pos: [%f, %f, %f]", camPos.x, camPos.y, camPos.z);
        ImGui::DragFloat3("Light[0]", &lights[0].position[0], 1.0f, -100.0f, 100.0f);

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vulkan::endRendering(cmd);

    // transfer to present
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

void Renderer::recordSkyboxPass(VkCommandBuffer cmd)
{
    vulkan::Swapchain &swapchain = graphics->getSwapchain();

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

    float color[4] = {0.3, 0.0, 3.0, 0.3};
    vulkan::beginDebugLabel(cmd, "Skybox pass", color);

    vulkan::beginRendering(cmd, &colorAttachment, 1, nullptr, extent);

    vulkan::setViewport(cmd, 0.0f, 0.0f, extent.width, extent.height);
    vulkan::setScissor(cmd, extent);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.skybox);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &set, 0, nullptr);

    //
    // Draw
    //
    auto &scene = scenes["cube"];
    for (auto &mesh : scene.meshes) {
        vkCmdBindIndexBuffer(cmd, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

        GPUMeshPushConstant pc;
        pc.address = mesh.vertexBuffer.address;
        pc.worldMatrix = mesh.matrix * scene.transform.getModelMatrix();
        pc.materialIdx = mesh.materialIdx;

        vkCmdPushConstants(cmd, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
        vkCmdDrawIndexed(cmd, mesh.indexCount, 1, 0, 0, 0);
    }

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

void Renderer::updateDynamicData()
{
    for (auto &light : lights) {
        mat4 projection = math::perspective(glm::radians(45.0f), 1.0f, 1.0f, 100.0f);
        mat4 view = glm::lookAt(light.position, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
        mat4 mvp = projection * view;
        light.mvp = mvp;
    }

    if (lightsBuffer.buffer != VK_NULL_HANDLE)
        memcpy(lightsBuffer.info.pMappedData, lights.data(), lightsBuffer.size);

    GlobalUBO ubo = {};
    ubo.projection = camera->getProjection();
    ubo.view = camera->getViewMatrix();
    ubo.cameraPosAndLightNum = vec4(camera->getPosition(), lights.size());
    memcpy(uboBuffer.info.pMappedData, &ubo, sizeof(ubo));
}

void Renderer::createResources()
{
    createBuffers();
    createColorImage();
    createDepthImage();
    createShadowMap();
    createSkybox();
    createDescriptors();
    createPipelines();
}

void Renderer::createBuffers()
{
    const VkDevice device = graphics->getDevice();

    graphics->createBuffer(&uboBuffer, sizeof(GlobalUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    vulkan::setDebugName(device, reinterpret_cast<uint64_t>(uboBuffer.buffer), VK_OBJECT_TYPE_BUFFER, "Global UBO");

    if (materials.size() > 0) {
        graphics->createBuffer(&materialsBuffer, materials.size() * sizeof(Material), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        vulkan::setDebugName(device, reinterpret_cast<uint64_t>(materialsBuffer.buffer), VK_OBJECT_TYPE_BUFFER, "Materials buffer");
        memcpy(materialsBuffer.info.pMappedData, materials.data(), materialsBuffer.size);
    }

    if (lights.size() > 0) {
        graphics->createBuffer(&lightsBuffer, lights.size() * sizeof(Light), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        vulkan::setDebugName(device, reinterpret_cast<uint64_t>(lightsBuffer.buffer), VK_OBJECT_TYPE_BUFFER, "Lights buffer");
        memcpy(lightsBuffer.info.pMappedData, lights.data(), lightsBuffer.size);
    }
}

void Renderer::createColorImage()
{
    const VkExtent2D extent = graphics->getSwapchain().getExtent();

    graphics->createImage(
        &colorImage, extent.width, extent.height, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_VIEW_TYPE_2D,
        VK_IMAGE_ASPECT_COLOR_BIT, graphics->getSampleCount()
    );
    vulkan::setDebugName(graphics->getDevice(), reinterpret_cast<uint64_t>(colorImage.image), VK_OBJECT_TYPE_IMAGE, "Color image");
}

void Renderer::createDepthImage()
{
    const VkExtent2D extent = graphics->getSwapchain().getExtent();

    graphics->createImage(
        &depthImage, extent.width, extent.height, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_FORMAT_D32_SFLOAT, VK_IMAGE_VIEW_TYPE_2D,
        VK_IMAGE_ASPECT_DEPTH_BIT, graphics->getSampleCount()
    );
    vulkan::setDebugName(graphics->getDevice(), reinterpret_cast<uint64_t>(depthImage.image), VK_OBJECT_TYPE_IMAGE, "Depth image");
}

void Renderer::createShadowMap()
{
    graphics->createImage(
        &shadowMap, shadowMapSize, shadowMapSize, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_FORMAT_D32_SFLOAT, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT
    );
    vulkan::setDebugName(graphics->getDevice(), reinterpret_cast<uint64_t>(shadowMap.image), VK_OBJECT_TYPE_IMAGE, "Shadowmap image");
}

void Renderer::createSkybox()
{
    graphics->createCubemapImage(&skybox, "assets/textures/skybox", VK_FORMAT_R8G8B8A8_SRGB);
    vulkan::setDebugName(graphics->getDevice(), reinterpret_cast<uint64_t>(skybox.image), VK_OBJECT_TYPE_IMAGE, "Skybox image");
}

void Renderer::createDescriptors()
{
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},                                         // ubo
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2},                                         // materials, lights
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)textures.size() + 1 + 1}, // textures + shadowmap + skybox
    };

    pool = graphics->createDescriptorPool(poolSizes);

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT},    // ubo
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT},                                 // materials
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT},                                 // lights
        {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)textures.size(), VK_SHADER_STAGE_FRAGMENT_BIT}, // textures
        {4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT},                         // shadowmap
        {5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT},                         // skybox
    };

    setLayout = graphics->createDescriptorSetLayout(bindings.data(), bindings.size(), nullptr);
    set = graphics->createDescriptorSet(pool, setLayout);

    std::vector<VkDescriptorImageInfo> textureInfos(textures.size());
    for (size_t i = 0; i < textures.size(); i++) {
        textureInfos[i].imageView = textures[i].view;
        textureInfos[i].sampler = textures[i].sampler;
        textureInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    // Update descriptors, if necessary
    vulkan::DescriptorWriter writer;
    if (uboBuffer.buffer != VK_NULL_HANDLE)
        writer.write(0, uboBuffer.buffer, uboBuffer.size, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    if (materialsBuffer.buffer != VK_NULL_HANDLE)
        writer.write(1, materialsBuffer.buffer, materialsBuffer.size, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    if (lightsBuffer.buffer != VK_NULL_HANDLE)
        writer.write(2, lightsBuffer.buffer, lightsBuffer.size, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

    if (textures.size() > 0)
        writer.write(3, textureInfos.data(), textureInfos.size(), VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    if (shadowMap.view != VK_NULL_HANDLE)
        writer.write(4, shadowMap.view, shadowMap.sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    if (skybox.image != VK_NULL_HANDLE)
        writer.write(5, skybox.view, skybox.sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    writer.update(graphics->getDevice(), set);
}

void Renderer::createPipelines()
{
    const VkDevice device = graphics->getDevice();

    // create pipeline layout
    VkPushConstantRange pushConstant = {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GPUMeshPushConstant)};
    pipelineLayout = graphics->createPipelineLayout(&setLayout, &pushConstant);

    // mesh pipeline
    vulkan::PipelineBuilder builder;
    builder.setPipelineLayout(pipelineLayout);
    builder.setShader(shaders["mesh.vert.spv"], VK_SHADER_STAGE_VERTEX_BIT);
    builder.setShader(shaders["mesh.frag.spv"], VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.setDepthTest(true);
    builder.setCulling(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    builder.setPolygonMode(VK_POLYGON_MODE_FILL);
    builder.setMultisampleCount(graphics->getSampleCount());
    pipelines.scene = builder.build(device, 1);

    builder.clear();

    // shadow pipeline
    builder.setPipelineLayout(pipelineLayout);
    builder.setShader(shaders["stub.vert.spv"], VK_SHADER_STAGE_VERTEX_BIT);
    builder.setDepthTest(true);
    builder.setCulling(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    builder.setPolygonMode(VK_POLYGON_MODE_FILL);
    pipelines.shadow = builder.build(device, 0);

    builder.clear();

    // skybox pipeline
    builder.setPipelineLayout(pipelineLayout);
    builder.setShader(shaders["skybox.vert.spv"], VK_SHADER_STAGE_VERTEX_BIT);
    builder.setShader(shaders["skybox.frag.spv"], VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.setCulling(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
    builder.setPolygonMode(VK_POLYGON_MODE_FILL);
    builder.setMultisampleCount(graphics->getSampleCount());
    pipelines.skybox = builder.build(device, 1);

    // debug
    vulkan::setDebugName(device, reinterpret_cast<uint64_t>(pipelines.scene), VK_OBJECT_TYPE_PIPELINE, "scene pipeline");
    vulkan::setDebugName(device, reinterpret_cast<uint64_t>(pipelines.shadow), VK_OBJECT_TYPE_PIPELINE, "shadow pipeline");
}

} // namespace rebirth