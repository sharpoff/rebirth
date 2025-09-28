#include <rebirth/graphics/renderer.h>

#include <rebirth/math/projection.h>

#include <rebirth/graphics/gltf.h>
#include <rebirth/graphics/primitives.h>
#include <rebirth/graphics/render_settings.h>
#include <rebirth/graphics/vulkan/descriptor_writer.h>
#include <rebirth/graphics/vulkan/pipeline_builder.h>
#include <rebirth/graphics/vulkan/swapchain.h>
#include <rebirth/graphics/vulkan/util.h>
#include <rebirth/types/scene_draw_data.h>

#include <rebirth/math/frustum_culling.h>
#include <rebirth/types/scene.h>
#include <rebirth/util/logger.h>

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

    const VkDevice device = g_graphics.getDevice();
    vkDeviceWaitIdle(device);

    g_resourceManager.destroy();

    vkDestroyQueryPool(device, queryPool, nullptr);

    destroyPipelines();

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
                    .boundingSphere = calculateBoundingSphere(g_resourceManager.getMesh(meshId))});
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
                .boundingSphere = calculateBoundingSphere(g_resourceManager.getMesh(meshId))});
    }
}

void Renderer::drawMesh(MeshID meshId, Transform transform)
{
    ZoneScoped;

    meshDraws.push_back(
        MeshDraw{
            .meshId = meshId,
            .transform = transform.getModelMatrix(),
            .boundingSphere = calculateBoundingSphere(g_resourceManager.getMesh(meshId))});
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

    sceneData.projection = camera.projection;
    sceneData.view = camera.view;
    sceneData.cameraPosAndLightNum = vec4(camera.position, lights.size());
    sceneData.shadowMapId = shadowMapId;
    memcpy(sceneDataBuffer.info.pMappedData, &sceneData, sizeof(sceneData));
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

    opaqueDraws.reserve(meshDraws.size());

    cullMeshDraws(camera.view * camera.projection);
    sortMeshDraws(camera.position);

    //
    // Create and begin command buffer
    //
    const VkCommandBuffer cmd = g_graphics.beginCommandBuffer();
    if (cmd == VK_NULL_HANDLE) {
        // Don't present - recreating swapchain
        return;
    }

    bool supportTimestamps = g_graphics.supportTimestamps();

    if (supportTimestamps) {
        g_graphics.resetQueryPool(cmd, queryPool, 0, timestamps.size());

        // write start timestamp
        g_graphics.writeTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, 0);
    }

    vulkan::Swapchain &swapchain = g_graphics.getSwapchain();
    const VkImage &swapchainImage = swapchain.getImage();

    //
    // Render passes start
    //

    // transfer swapchain image to color attachment
    VkImageMemoryBarrier swapchainBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapchainImage,
        .subresourceRange = colorSubresource};

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &swapchainBarrier);

    vkCmdBindIndexBuffer(cmd, g_resourceManager.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    //
    // Clear Pass
    //
    {
        ZoneScopedN("Clear Pass");
        TracyVkZone(g_graphics.getTracyContext(), cmd, "Clear Pass");

        clearPass(cmd);
    }

    //
    // Shadow Pass
    //
    if (g_renderSettings.drawShadows && !opaqueDraws.empty()) {
        ZoneScopedN("Shadow Pass");
        TracyVkZone(g_graphics.getTracyContext(), cmd, "Shadow Pass");

        shadowPass(cmd);
    }

    //
    // Mesh Pass
    //
    if (g_renderSettings.drawMeshes && !opaqueDraws.empty()) {
        ZoneScopedN("Mesh Pass");
        TracyVkZone(g_graphics.getTracyContext(), cmd, "Mesh Pass");

        meshPass(cmd);
    }

    //
    // Skybox Pass
    //
    if (g_renderSettings.drawSkybox) {
        ZoneScopedN("Skybox Pass");
        TracyVkZone(g_graphics.getTracyContext(), cmd, "Skybox Pass");

        skyboxPass(cmd);
    }

    //
    // Imgui Pass
    //
    {
        ZoneScopedN("ImGui Pass");
        TracyVkZone(g_graphics.getTracyContext(), cmd, "ImGui Pass");

        imGuiPass(cmd);
    }

    // transfer swapchain image to present
    VkImageMemoryBarrier presentBarier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapchainImage,
        .subresourceRange = colorSubresource};

    vkCmdPipelineBarrier(
        cmd,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &presentBarier);

    //
    // Render passes end
    //

    if (supportTimestamps) {
        // write end timestamp
        g_graphics.writeTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, 1);
    }

    TracyVkCollect(g_graphics.getTracyContext(), cmd);

    // Submit
    g_graphics.submitCommandBuffer(cmd);

    if (supportTimestamps) {
        // get timestamp result
        vkGetQueryPoolResults(g_graphics.getDevice(), queryPool, 0, timestamps.size(), timestamps.size() * sizeof(uint64_t), timestamps.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    }

    debugDrawVertices.clear();
    meshDraws.clear();
    opaqueDraws.clear();
    g_renderSettings.drawCount = 0;
}

void Renderer::cullMeshDraws(mat4 viewProj)
{
    ZoneScoped;

    for (size_t i = 0; i < meshDraws.size(); i++) {
        // if (isSphereVisible(meshDraws[i].boundingSphere, viewProj, meshDraws[i].transform)) {
        opaqueDraws.push_back(i);
        // }
    }
}

void Renderer::sortMeshDraws(vec3 cameraPos)
{
    ZoneScoped;

    if (opaqueDraws.empty())
        return;

    std::sort(opaqueDraws.begin(), opaqueDraws.end(), [&](const auto &i1, const auto &i2) {
        float dist1 = glm::length(cameraPos - math::getPosition(meshDraws[i1].transform));
        float dist2 = glm::length(cameraPos - math::getPosition(meshDraws[i2].transform));

        return dist1 < dist2;
    });
}

std::unordered_map<std::string, VkShaderModule> Renderer::loadShaderModules(std::filesystem::path directory)
{
    ZoneScoped;

    std::unordered_map<std::string, VkShaderModule> shaders;

    const VkDevice device = g_graphics.getDevice();
    for (auto &entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            shaders[entry.path().filename()] = vulkan::util::loadShaderModule(device, entry.path());
        }
    }

    return shaders;
}

void Renderer::createPipelines()
{
    ZoneScoped;

    const VkDevice device = g_graphics.getDevice();
    DescriptorManager &descriptorManager = g_graphics.getDescriptorManager();
    const VkFormat colorFormat = g_graphics.getSwapchain().getSurfaceFormat().format;

    std::unordered_map<std::string, VkShaderModule> shaders = loadShaderModules("build/shaders");

    //
    // Create pipeline layouts
    //
    {
        // shadow pipeline layout
        VkPushConstantRange pushConstant = {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ShadowPassPC)};
        pipelineLayouts["shadow"] = g_graphics.createPipelineLayout(&descriptorManager.getSetLayout(), &pushConstant);
    }

    {
        // mesh pipeline layout
        VkPushConstantRange pushConstant = {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(MeshPassPC)};
        pipelineLayouts["mesh"] = g_graphics.createPipelineLayout(&descriptorManager.getSetLayout(), &pushConstant);
    }

    {
        // skybox pipeline layout
        VkPushConstantRange pushConstant = {VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SkyboxPassPC)};
        pipelineLayouts["skybox"] = g_graphics.createPipelineLayout(&descriptorManager.getSetLayout(), &pushConstant);
    }

    //
    // Create pipelines
    //
    {
        // shadow pipeline
        PipelineBuilder builder;
        builder.setPipelineLayout(pipelineLayouts["shadow"]);
        builder.setShader(shaders["shadow.vert.spv"], VK_SHADER_STAGE_VERTEX_BIT); // default fragment shader
        builder.setDepthTest(VK_TRUE, VK_TRUE);
        builder.setCulling(VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        builder.setPolygonMode(VK_POLYGON_MODE_FILL);
        pipelines["shadow"] = builder.build(device, {});

        vulkan::util::setDebugName(device, (uint64_t)pipelines["shadow"], VK_OBJECT_TYPE_PIPELINE, "Shadow pipeline");
    }

    {
        // mesh pipeline
        PipelineBuilder builder;
        builder.setPipelineLayout(pipelineLayouts["mesh"]);
        builder.setShader(shaders["mesh.vert.spv"], VK_SHADER_STAGE_VERTEX_BIT);
        builder.setShader(shaders["mesh.frag.spv"], VK_SHADER_STAGE_FRAGMENT_BIT);
        builder.setDepthTest(VK_TRUE, VK_TRUE);
        builder.setCulling(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        builder.setPolygonMode(VK_POLYGON_MODE_FILL);
        builder.setMultisampleCount(g_graphics.getSampleCount());
        pipelines["mesh"] = builder.build(device, {colorFormat});

        vulkan::util::setDebugName(device, (uint64_t)pipelines["mesh"], VK_OBJECT_TYPE_PIPELINE, "Mesh pipeline");
    }

    {
        // wireframe pipeline
        PipelineBuilder builder;
        builder.setPipelineLayout(pipelineLayouts["mesh"]);
        builder.setShader(shaders["color.vert.spv"], VK_SHADER_STAGE_VERTEX_BIT);
        builder.setShader(shaders["color.frag.spv"], VK_SHADER_STAGE_FRAGMENT_BIT);
        builder.setDepthTest(VK_TRUE, VK_TRUE);
        builder.setCulling(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        builder.setPolygonMode(VK_POLYGON_MODE_LINE);
        builder.setMultisampleCount(g_graphics.getSampleCount());
        pipelines["wireframe"] = builder.build(device, {colorFormat, colorFormat});

        vulkan::util::setDebugName(device, (uint64_t)pipelines["wireframe"], VK_OBJECT_TYPE_PIPELINE, "Wireframe pipeline");
    }

    {
        // skybox pipeline
        PipelineBuilder builder;
        builder.setPipelineLayout(pipelineLayouts["skybox"]);
        builder.setShader(shaders["skybox.vert.spv"], VK_SHADER_STAGE_VERTEX_BIT);
        builder.setShader(shaders["skybox.frag.spv"], VK_SHADER_STAGE_FRAGMENT_BIT);
        builder.setCulling(VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        builder.setDepthTest(VK_TRUE, VK_FALSE, VK_COMPARE_OP_EQUAL);
        builder.setMultisampleCount(g_graphics.getSampleCount());
        pipelines["skybox"] = builder.build(device, {colorFormat});

        vulkan::util::setDebugName(device, (uint64_t)pipelines["skybox"], VK_OBJECT_TYPE_PIPELINE, "Skybox pipeline");
    }

    for (auto &[_, shader] : shaders) {
        vkDestroyShaderModule(device, shader, nullptr);
    }
}

void Renderer::destroyPipelines()
{
    ZoneScoped;

    const VkDevice device = g_graphics.getDevice();

    for (auto &[_, pipeline] : pipelines) {
        vkDestroyPipeline(device, pipeline, nullptr);
    }

    for (auto &[_, pipelineLayout] : pipelineLayouts) {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }
}

void Renderer::createResources()
{
    ZoneScoped;

    // shadow map
    {
        ImageCreateInfo createInfo = {
            .width = SHADOW_MAP_SIZE,
            .height = SHADOW_MAP_SIZE,
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
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
    updateDescriptorSet();
}

void Renderer::createBuffers()
{
    ZoneScoped;

    const VkDevice device = g_graphics.getDevice();

    // scene data
    {
        BufferCreateInfo createInfo = {
            .size = sizeof(SceneDrawData),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        };

        g_graphics.createBuffer(sceneDataBuffer, createInfo);
        vulkan::util::setDebugName(device, reinterpret_cast<uint64_t>(sceneDataBuffer.buffer), VK_OBJECT_TYPE_BUFFER, "Scene Data buffer");
    }

    // materials
    {
        auto &materials = g_resourceManager.materials;
        BufferCreateInfo createInfo = {
            .size = materials.size() * sizeof(Material),
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        };

        g_graphics.createBuffer(materialsBuffer, createInfo);
        vulkan::util::setDebugName(device, reinterpret_cast<uint64_t>(materialsBuffer.buffer), VK_OBJECT_TYPE_BUFFER, "Materials buffer");
        memcpy(materialsBuffer.info.pMappedData, materials.data(), materialsBuffer.size);
    }

    // lights
    {
        auto &lights = g_resourceManager.lights;

        BufferCreateInfo createInfo = {
            .size = lights.size() * sizeof(Light),
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        };

        g_graphics.createBuffer(lightsBuffer, createInfo);
        vulkan::util::setDebugName(device, reinterpret_cast<uint64_t>(lightsBuffer.buffer), VK_OBJECT_TYPE_BUFFER, "Lights buffer");
        memcpy(lightsBuffer.info.pMappedData, lights.data(), lightsBuffer.size);
    }
}

void Renderer::updateDescriptorSet()
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
    destroyPipelines();
    createPipelines();
}