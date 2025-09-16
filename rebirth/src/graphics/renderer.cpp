#include <rebirth/graphics/renderer.h>

#include <rebirth/math/projection.h>

#include <rebirth/graphics/primitives.h>
#include <rebirth/graphics/render_settings.h>
#include <rebirth/graphics/vulkan/descriptor_writer.h>
#include <rebirth/graphics/vulkan/pipeline_builder.h>
#include <rebirth/graphics/vulkan/swapchain.h>
#include <rebirth/graphics/vulkan/util.h>
#include <rebirth/types/scene_draw_data.h>

#include <rebirth/types/scene.h>
#include <rebirth/util/logger.h>

#include <rebirth/graphics/gltf.h>

#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

void Renderer::initialize(SDL_Window *window)
{
    ZoneScopedN("Renderer initialize");

    assert(window);
    this->window = window;

    g_graphics.initialize(window);

    // query
    queryPool = g_graphics.createQueryPool(VK_QUERY_TYPE_TIMESTAMP, timestamps.size());

    // load common mesh primitives
    cubeModelId = generateCube();
    sphereModelId = generateUVSphere(1.0f);

    Model cylinderModel;
    if (!gltf::loadModel(cylinderModel, "assets/models/cylinder.glb")) {
        ::util::logError("Falied to load cylinder primitive!");
        exit(EXIT_FAILURE);
    }
    cylinderModelId = g_resourceManager.addModel(cylinderModel);

    createPipelines();

    ImageCreateInfo createInfo{};
    Image checkerboard;
    g_graphics.createImageFromFile(checkerboard, createInfo, "assets/textures/checkerboard.png");
    defaultImageId = g_resourceManager.addImage(checkerboard);

    Material defaultMaterial;
    defaultMaterial.baseColorId = ID(defaultImageId);
    defaultMaterialId = g_resourceManager.addMaterial(defaultMaterial);
}

void Renderer::shutdown()
{
    ZoneScopedN("Renderer shutdown");

    vkDeviceWaitIdle(g_graphics.getDevice());

    g_resourceManager.destroy();

    vkDestroyQueryPool(g_graphics.getDevice(), queryPool, nullptr);

    shadowPipeline.destroy();
    meshPipeline.destroy();
    skyboxPipeline.destroy();

    g_graphics.destroyBuffer(sceneDataBuffer);
    g_graphics.destroyBuffer(materialsBuffer);
    g_graphics.destroyBuffer(lightsBuffer);

    g_graphics.destroy();
}

void Renderer::addLight(Light light) { g_resourceManager.addLight(light); }

void Renderer::drawScene(Scene &scene, Transform transform)
{
    ZoneScoped;

    std::function<void(SceneNode &)> nodeDraw = [&](SceneNode &node) {
        for (MeshID meshId : node.model.meshes) {
            meshDraws.push_back(
                MeshDraw{
                    .meshId = meshId,
                    .transform = transform.getModelMatrix() * scene.getNodeWorldMatrix(&node),
                    .boundingSphere = SphereBounding(),
                });
        }

        for (auto &child : node.children) {
            nodeDraw(child);
        }
    };

    for (auto &node : scene.nodes) {
        nodeDraw(node);
    }
}

void Renderer::drawModel(ModelID modelId, Transform transform)
{
    ZoneScoped;

    Model &model = g_resourceManager.getModel(modelId);

    for (MeshID meshId : model.meshes) {
        meshDraws.push_back(
            MeshDraw{
                .meshId = meshId,
                .transform = transform.getModelMatrix(),
                .boundingSphere = SphereBounding(),
            });
    }
}

void Renderer::drawMesh(MeshID meshId, Transform transform)
{
    ZoneScoped;

    meshDraws.push_back(
        MeshDraw{
            .meshId = meshId,
            .transform = transform.getModelMatrix(),
            .boundingSphere = SphereBounding(),
        });
}

void Renderer::present(Camera &camera)
{
    ZoneScoped;

    if (!prepared) {
        createResources();
        prepared = true;
    }

    // TODO: create and update global joints buffer
    updateDynamicData(camera);

    // batchMeshDraws();
    // cullDrawBatches();

    //
    // Create and begin command buffer
    //
    const VkCommandBuffer cmd = g_graphics.beginCommandBuffer();

    if (cmd == VK_NULL_HANDLE) {
        // Don't present - recreating swapchain
        return;
    }

    if (g_graphics.supportTimestamps()) {
        g_graphics.resetQueryPool(cmd, queryPool, 0, timestamps.size());

        // write start timestamp
        g_graphics.writeTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, 0);
    }

    vulkan::Swapchain &swapchain = g_graphics.getSwapchain();
    const VkImage &swapchainImage = swapchain.getImage();
    const Image &colorImage = g_graphics.getColorImage();
    const Image &colorImageOneSample = g_graphics.getColorImageOneSample();
    const Image &depthImage = g_graphics.getDepthImage();
    const Image &shadowMap = g_resourceManager.getImage(shadowMapId);

    VkImageSubresourceRange colorSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1};
    VkImageSubresourceRange depthSubresource = colorSubresource;
    depthSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    // transfer one sample color image to color attachment
    vulkan::util::imageBarrier(cmd, colorImageOneSample.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, colorSubresource);

    // transfer multisampled image to color attachment
    vulkan::util::imageBarrier(cmd, colorImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, colorSubresource);

    // transfer swapchain image to color attachment
    vulkan::util::imageBarrier(cmd, swapchainImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, colorSubresource);

    // transfer to depth attachment
    vulkan::util::imageBarrier(cmd, depthImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, depthSubresource);

    //
    // Render passes start
    //

    Frustum frustum(camera);

    // Skybox Pass
    {
        ZoneScopedN("Skybox Pass");
        TracyVkZone(g_graphics.getTracyContext(), cmd, "Skybox Pass");

        skyboxPipeline.beginFrame(cmd);
        if (g_renderSettings.skybox) {
            skyboxPipeline.drawSkybox(cmd, skyboxId);
        }
        skyboxPipeline.endFrame(cmd);
    }

    // transfer shadowmap to depth attachment
    vulkan::util::imageBarrier(cmd, shadowMap.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, depthSubresource);

    // Shadow Pass
    if (!meshDraws.empty()) {
        ZoneScopedN("Shadow Pass");
        TracyVkZone(g_graphics.getTracyContext(), cmd, "Shadow Pass");

        shadowPipeline.beginFrame(cmd, shadowMap.view);

        if (g_renderSettings.shadows) {
            auto &lights = g_resourceManager.lights;
            for (auto &light : lights)
                shadowPipeline.draw(cmd, meshDraws, light.mvp);
        }

        shadowPipeline.endFrame(cmd, g_renderSettings.debugShadowMap);
    }

    // transfer shadowmap to fragment shader read
    vulkan::util::imageBarrier(cmd, shadowMap.image, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, depthSubresource);

    // Mesh Pass
    if (!g_renderSettings.debugShadowMap && !meshDraws.empty()) {
        ZoneScopedN("Mesh Pass");
        TracyVkZone(g_graphics.getTracyContext(), cmd, "Mesh Pass");

        meshPipeline.beginFrame(cmd, g_renderSettings.wireframe);
        meshPipeline.draw(cmd, frustum, meshDraws);
        meshPipeline.endFrame(cmd);
    }

    // transfer color image to fragment shader
    vulkan::util::imageBarrier(cmd, colorImageOneSample.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, colorSubresource);

    // Imgui Pass
    {
        ZoneScopedN("ImGui Pass");
        TracyVkZone(g_graphics.getTracyContext(), cmd, "ImGui Pass");

        imguiPipeline.beginFrame(cmd);
        imguiPipeline.draw();
        imguiPipeline.endFrame(cmd);
    }

    // transfer swapchain image to present
    vulkan::util::imageBarrier(cmd, swapchainImage, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0, colorSubresource);

    //
    // Render passes end
    //

    if (g_graphics.supportTimestamps()) {
        // write end timestamp
        g_graphics.writeTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, 1);
    }

    TracyVkCollect(g_graphics.getTracyContext(), cmd);

    g_graphics.submitCommandBuffer(cmd);

    if (g_graphics.supportTimestamps()) {
        // get timestamp result
        vkGetQueryPoolResults(g_graphics.getDevice(), queryPool, 0, timestamps.size(), timestamps.size() * sizeof(uint64_t), timestamps.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    }

    meshDraws.clear();
    drawBatches.clear();
    g_renderSettings.drawCount = 0;
}

void Renderer::batchMeshDraws()
{
    if (meshDraws.empty())
        return;

    DrawBatch firstBatch;
    firstBatch.first = 0;
    firstBatch.count = 1;
    firstBatch.meshId = meshDraws[0].meshId;

    drawBatches.push_back(firstBatch);

    for (auto &meshDraw : meshDraws) {
        if (drawBatches.back().meshId == meshDraw.meshId) {
            // same batch
            drawBatches.back().count++;
        } else {
            // new batch
            DrawBatch firstBatch;
            firstBatch.first = 0;
            firstBatch.count = 1;
            firstBatch.meshId = meshDraw.meshId;

            drawBatches.push_back(firstBatch);
        }
    }
}

void Renderer::cullDrawBatches()
{
    // todo: add compute shader culling
}

void Renderer::createPipelines()
{
    ZoneScoped;

    meshPipeline.initialize();
    shadowPipeline.initialize();
    skyboxPipeline.initialize(cubeModelId);
}

void Renderer::updateDynamicData(Camera &camera)
{
    ZoneScoped;

    auto &lights = g_resourceManager.lights;
    for (auto &light : lights) {
        if (light.type == LightType::Point) {
            mat4 projection = math::perspective(glm::radians(45.0f), 1.0f, 1.0f, 100.0f);
            mat4 view = glm::lookAt(light.position, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
            mat4 mvp = projection * view;
            light.mvp = mvp;
        } else if (light.type == LightType::Directional) {
            // mat4 projection = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 100.0f);
            mat4 projection = math::perspectiveInf(glm::radians(45.0f), 1.0f, 1.0f);
            mat4 view = glm::lookAt(vec3(-4, 100, -4), vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
            mat4 mvp = projection * view;
            light.mvp = mvp;
        }
    }
    memcpy(lightsBuffer.info.pMappedData, lights.data(), lightsBuffer.size);

    SceneDrawData sceneData = {};
    sceneData.projection = camera.projection;
    sceneData.view = camera.view;
    sceneData.cameraPosAndLightNum = vec4(camera.position, lights.size());
    sceneData.shadowMapId = shadowMapId;
    memcpy(sceneDataBuffer.info.pMappedData, &sceneData, sizeof(sceneData));
}

void Renderer::createResources()
{
    ZoneScoped;

    // shadow map
    {
        ImageCreateInfo createInfo = {
            .width = shadowPipeline.getShadowMapSize(),
            .height = shadowPipeline.getShadowMapSize(),
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .format = VK_FORMAT_D32_SFLOAT,
            .aspect = VK_IMAGE_ASPECT_DEPTH_BIT,
        };

        vulkan::Image shadowMap;
        g_graphics.createImage(shadowMap, createInfo, false);
        vulkan::util::setDebugName(g_graphics.getDevice(), reinterpret_cast<uint64_t>(shadowMap.image), VK_OBJECT_TYPE_IMAGE, "Shadowmap image");
        shadowMapId = g_resourceManager.addImage(shadowMap);
    }

    // skybox
    {
        ImageCreateInfo createInfo{};
        createInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        createInfo.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        createInfo.arrayLayers = 6;
        createInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        createInfo.addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

        vulkan::Image skybox;
        g_graphics.createCubemapImage(skybox, createInfo, "assets/textures/skybox");
        vulkan::util::setDebugName(g_graphics.getDevice(), reinterpret_cast<uint64_t>(skybox.image), VK_OBJECT_TYPE_IMAGE, "Skybox image");
        skyboxId = g_resourceManager.addImage(skybox);
    }

    // vertices/indices
    {
        g_resourceManager.createVertexBuffer();
        g_resourceManager.createIndexBuffer();
        g_resourceManager.createJointMatricesBuffer();
    }

    createBuffers();

    updateDescriptors();
}

void Renderer::createBuffers()
{
    ZoneScoped;

    const VkDevice device = g_graphics.getDevice();

    // scene data
    BufferCreateInfo createInfo = {
        .size = sizeof(SceneDrawData),
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    };

    g_graphics.createBuffer(sceneDataBuffer, createInfo);
    vulkan::util::setDebugName(device, reinterpret_cast<uint64_t>(sceneDataBuffer.buffer), VK_OBJECT_TYPE_BUFFER, "Scene Data buffer");

    // materials
    auto &materials = g_resourceManager.materials;

    createInfo.size = materials.size() * sizeof(Material);
    createInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    g_graphics.createBuffer(materialsBuffer, createInfo);
    vulkan::util::setDebugName(device, reinterpret_cast<uint64_t>(materialsBuffer.buffer), VK_OBJECT_TYPE_BUFFER, "Materials buffer");
    memcpy(materialsBuffer.info.pMappedData, materials.data(), materialsBuffer.size);

    // lights
    auto &lights = g_resourceManager.lights;

    createInfo.size = lights.size() * sizeof(Light);
    createInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    g_graphics.createBuffer(lightsBuffer, createInfo);
    vulkan::util::setDebugName(device, reinterpret_cast<uint64_t>(lightsBuffer.buffer), VK_OBJECT_TYPE_BUFFER, "Lights buffer");
    memcpy(lightsBuffer.info.pMappedData, lights.data(), lightsBuffer.size);
}

void Renderer::updateDescriptors()
{
    ZoneScoped;

    // Update descriptors, if necessary
    DescriptorWriter writer;

    auto &textures = g_resourceManager.images;
    for (size_t i = 0; i < textures.size(); i++) {
        writer.write(TEXTURES_BINDING, textures[i].view, textures[i].sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, i);
    }

    writer.write(SCENE_DATA_BINDING, sceneDataBuffer.buffer, sceneDataBuffer.size, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
    writer.write(MATERIALS_BINDING, materialsBuffer.buffer, materialsBuffer.size, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    writer.write(LIGHTS_BINDING, lightsBuffer.buffer, lightsBuffer.size, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    writer.write(VERTEX_BINDING, g_resourceManager.vertexBuffer.buffer, g_resourceManager.vertexBuffer.size, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);

    writer.update(g_graphics.getDevice(), g_graphics.getDescriptorManager().getSet());
}

void Renderer::reloadShaders()
{
    ZoneScoped;

    ::util::logInfo("Reloading shaders.");

    vkDeviceWaitIdle(g_graphics.getDevice());
    createPipelines();
}