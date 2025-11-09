#include "graphics/renderer.h"

#include "core/draw_mask.h"
#include "core/engine_stats.h"
#include "core/material.h"
#include "core/resource_manager.h"
#include "core/scene_draw_data.h"
#include "graphics/gltf.h"
#include "graphics/vulkan/descriptor_writer.h"
#include "graphics/vulkan/pipeline_builder.h"
#include "graphics/vulkan/resources.h"
#include "graphics/vulkan/swapchain.h"
#include "graphics/vulkan/util.h"
#include "game/entity.h"

#include "math/math.h"
#include "util/logger.h"

#include "tracy/Tracy.hpp"
#include "tracy/TracyVulkan.hpp"

void Renderer::initialize(SDL_Window *window, EngineStats *engineStats)
{
    ZoneScopedN("Renderer initialize");

    assert(window);
    this->window = window;
    this->engineStats = engineStats;

    graphics.initialize(window);

    // query
    queryPool = graphics.createQueryPool(VK_QUERY_TYPE_TIMESTAMP, timestamps.size());

    createPipelines();

    loadDefaultResources();
}

void Renderer::shutdown()
{
    ZoneScopedN("Renderer shutdown");

    const VkDevice device = graphics.getDevice();
    vkDeviceWaitIdle(device);

    vkDestroyQueryPool(device, queryPool, nullptr);

    destroyPipelines();

    for (Image &image : ResourceManager::get()->getImages())
        graphics.destroyImage(image);

    graphics.destroyBuffer(sceneDataBuffer);
    graphics.destroyBuffer(materialsBuffer);
    graphics.destroyBuffer(lightsBuffer);

    graphics.destroyBuffer(vertexBuffer);
    graphics.destroyBuffer(indexBuffer);
    graphics.destroyBuffer(jointMatricesBuffer);

    graphics.destroy();

    LOGI("%s", "Renderer shutdown");
}

void Renderer::drawEntity(const Entity &entity, const uint32_t &drawMask)
{
    ZoneScoped;

    if (entity.getMeshID() < 0) return;

    meshDraws.push_back(
        MeshDraw{
            .meshId = entity.getMeshID(),
            .overrideMaterialId = entity.getOverrideMaterialId(),
            .drawMask = drawMask,
            .transform = entity.getTransform(),
            .boundingSphere = entity.getBounds(),
        });
}

void Renderer::drawMesh(const int32_t &meshId, const uint32_t &drawMask, const mat4 &transform, const int32_t &overrideMaterialId)
{
    ZoneScoped;

    if (meshId < 0) return;

    meshDraws.push_back(
        MeshDraw{
            .meshId = meshId,
            .overrideMaterialId = overrideMaterialId,
            .drawMask = drawMask,
            .transform = transform,
            .boundingSphere = math::calculateBoundingSphere(meshId, math::getScale(transform), math::getRotation(transform)),
        });
}

void Renderer::addMeshDraw(const MeshDraw &meshDraw)
{
    ZoneScoped;

    meshDraws.push_back(meshDraw);
}

void Renderer::updateDynamicData()
{
    ZoneScoped;

    auto &lights = ResourceManager::get()->getLights();
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

    sceneData.projection = camera->getProjection();
    sceneData.view = camera->getView();
    sceneData.cameraPosAndLightNum = vec4(camera->getPosition(), lights.size());
    sceneData.shadowMapIndex = ResourceManager::get()->getImageIndexByName("shadow_map");
    memcpy(sceneDataBuffer.info.pMappedData, &sceneData, sizeof(sceneData));
}

bool Renderer::present(Editor *editor)
{
    ZoneScoped;

    assert(editor);

    //
    // Create and begin command buffer
    //
    const VkCommandBuffer cmd = graphics.beginCommandBuffer();
    if (cmd == VK_NULL_HANDLE) {
        // Don't present - recreating swapchain
        clearFrameData();
        return false;
    }

    // TODO: create and update global joints buffer
    updateDynamicData();

    cullAndIndexMeshDraws();
    sortMeshDraws(opaqueDrawIndices);
    sortMeshDraws(transparentDrawIndices);
    sortMeshDraws(overlayDrawIndices);

    bool supportTimestamps = graphics.supportTimestamps();

    if (supportTimestamps) {
        graphics.resetQueryPool(cmd, queryPool, 0, timestamps.size());

        // write start timestamp
        graphics.writeTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, 0);
    }

    vulkan::Swapchain &swapchain = graphics.getSwapchain();
    const VkImage     &swapchainImage = swapchain.getImage();

    //
    // Render passes start
    //

    // transfer swapchain image to color attachment
    VkImageMemoryBarrier swapchainBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = swapchainImage,
        .subresourceRange = vulkan::colorSubresource};

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

    if (indexBuffer.buffer)
        vkCmdBindIndexBuffer(cmd, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    //
    // Clear Pass
    //
    {
        ZoneScopedN("Clear Pass");
        TracyVkZone(graphics.getTracyContext(), cmd, "Clear Pass");

        clearPass(cmd);
    }

    
    // Shadow Pass
    //
    if (ResourceManager::get()->getLightsSize() > 0 && !opaqueDrawIndices.empty()) {
        ZoneScopedN("Shadow Pass");
        TracyVkZone(graphics.getTracyContext(), cmd, "Shadow Pass");

        shadowPass(cmd);
    }

    //
    // Opaque mesh Pass
    //
    if (!opaqueDrawIndices.empty()) {
        ZoneScopedN("Mesh Pass");
        TracyVkZone(graphics.getTracyContext(), cmd, "Mesh Pass");

        opaqueMeshPass(cmd);
    }

    //
    // Transparent mesh Pass
    //
    if (!transparentDrawIndices.empty()) {
        ZoneScopedN("Mesh Pass");
        TracyVkZone(graphics.getTracyContext(), cmd, "Mesh Pass");

        transparentMeshPass(cmd);
    }

    //
    // Skybox Pass
    //
    {
        ZoneScopedN("Skybox Pass");
        TracyVkZone(graphics.getTracyContext(), cmd, "Skybox Pass");

        skyboxPass(cmd);
    }

    // Overlay pass
    {
        ZoneScopedN("Overlay Pass");
        TracyVkZone(graphics.getTracyContext(), cmd, "Overlay Pass");

        overlayPass(cmd);
    }

    //
    // Imgui Pass
    //
    {
        ZoneScopedN("ImGui Pass");
        TracyVkZone(graphics.getTracyContext(), cmd, "ImGui Pass");

        imGuiPass(cmd, editor);
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
        graphics.writeTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, 1);
    }

    TracyVkCollect(graphics.getTracyContext(), cmd);

    // Submit
    graphics.submitCommandBuffer(cmd);

    if (supportTimestamps) {
        // get timestamp result
        vkGetQueryPoolResults(graphics.getDevice(), queryPool, 0, timestamps.size(), timestamps.size() * sizeof(uint64_t), timestamps.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    }

    engineStats->timestampDeltaMs = getTimestampDeltaMs();
    clearFrameData();

    return true;
}

void Renderer::clearFrameData()
{
    meshDraws.clear();

    opaqueDrawIndices.clear();
    overlayDrawIndices.clear();
    transparentDrawIndices.clear();

    engineStats->drawCount = 0;
}

void Renderer::cullAndIndexMeshDraws()
{
    ZoneScoped;
    assert(camera);

    // const mat4 &viewProj = camera->getProjection() * camera->getView();

    for (size_t i = 0; i < meshDraws.size(); i++) {
        MeshDraw &draw = meshDraws[i];
        // FIXME: not working properly
        // if (!math::isSphereVisible(meshDraws[i].boundingSphere, viewProj, meshDraws[i].transform * ResourceManager::get()->getMeshByIndex(meshDraws[i].meshId)->transform))
        //     continue;

        if ((draw.drawMask & DrawMask::Overlay) == DrawMask::Overlay) {
            overlayDrawIndices.push_back(i);
        } else if ((draw.drawMask & DrawMask::Opaque) == DrawMask::Opaque) {
            opaqueDrawIndices.push_back(i);
        } else if ((draw.drawMask & DrawMask::Transparent) == DrawMask::Transparent) {
            transparentDrawIndices.push_back(i);
        }
    }
}

void Renderer::sortMeshDraws(eastl::vector<uint32_t> &draws)
{
    ZoneScoped;
    assert(camera);

    std::sort(draws.begin(), draws.end(), [&](const auto &i1, const auto &i2) {
        auto *mesh1 = ResourceManager::get()->getMeshByIndex(meshDraws[i1].meshId);
        auto *mesh2 = ResourceManager::get()->getMeshByIndex(meshDraws[i2].meshId);

        float dist1 = glm::length(camera->getPosition() - math::getPosition(mesh1->transform));
        float dist2 = glm::length(camera->getPosition() - math::getPosition(mesh2->transform));

        return dist1 > dist2;
    });
}

eastl::unordered_map<eastl::string, VkShaderModule> Renderer::loadShaderModules(std::filesystem::path directory)
{
    ZoneScoped;

    eastl::unordered_map<eastl::string, VkShaderModule> shaders;

    const VkDevice device = graphics.getDevice();
    for (auto &entry : std::filesystem::recursive_directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            shaders[entry.path().filename().c_str()] = vulkan::loadShaderModule(device, entry.path());
        }
    }

    return shaders;
}

void Renderer::createPipelines()
{
    ZoneScoped;

    const VkDevice     device = graphics.getDevice();
    DescriptorManager &descriptorManager = graphics.getDescriptorManager();
    const VkFormat     colorFormat = graphics.getSwapchain().getSurfaceFormat().format;

    eastl::unordered_map<eastl::string, VkShaderModule> shaders = loadShaderModules("shaders/bin");

    //
    // Create pipeline layouts
    //
    {
        // shadow pipeline layout
        VkPushConstantRange pushConstant = {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ShadowPassPC)};
        pipelineLayouts["shadow"] = graphics.createPipelineLayout(&descriptorManager.getSetLayout(), &pushConstant);
    }

    {
        // mesh pipeline layout
        VkPushConstantRange pushConstant = {VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(MeshPassPC)};
        pipelineLayouts["mesh"] = graphics.createPipelineLayout(&descriptorManager.getSetLayout(), &pushConstant);
    }

    {
        // skybox pipeline layout
        VkPushConstantRange pushConstant = {VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SkyboxPassPC)};
        pipelineLayouts["skybox"] = graphics.createPipelineLayout(&descriptorManager.getSetLayout(), &pushConstant);
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
    }

    {
        // opaque mesh pipeline
        PipelineBuilder builder;
        builder.setPipelineLayout(pipelineLayouts["mesh"]);
        builder.setShader(shaders["mesh.vert.spv"], VK_SHADER_STAGE_VERTEX_BIT);
        builder.setShader(shaders["mesh.frag.spv"], VK_SHADER_STAGE_FRAGMENT_BIT);
        builder.setDepthTest(VK_TRUE, VK_TRUE);
        builder.setCulling(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        builder.setPolygonMode(VK_POLYGON_MODE_FILL);
        builder.setMultisampleCount(graphics.getSampleCount());
        pipelines["mesh_opaque"] = builder.build(device, {colorFormat});
    }

    {
        // transparent mesh pipeline
        PipelineBuilder builder;
        builder.setPipelineLayout(pipelineLayouts["mesh"]);
        builder.setShader(shaders["mesh.vert.spv"], VK_SHADER_STAGE_VERTEX_BIT);
        builder.setShader(shaders["mesh.frag.spv"], VK_SHADER_STAGE_FRAGMENT_BIT);
        builder.setDepthTest(VK_TRUE, VK_TRUE);
        builder.setCulling(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        builder.setPolygonMode(VK_POLYGON_MODE_FILL);
        builder.setMultisampleCount(graphics.getSampleCount());
        builder.setBlendingAlphaBlend(); // alpha blending
        pipelines["mesh_transparent"] = builder.build(device, {colorFormat});
    }

    {
        // wireframe pipeline
        PipelineBuilder builder;
        builder.setPipelineLayout(pipelineLayouts["mesh"]);
        builder.setShader(shaders["mesh.vert.spv"], VK_SHADER_STAGE_VERTEX_BIT);
        builder.setShader(shaders["mesh.frag.spv"], VK_SHADER_STAGE_FRAGMENT_BIT);
        builder.setDepthTest(VK_TRUE, VK_TRUE);
        builder.setCulling(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        builder.setPolygonMode(VK_POLYGON_MODE_LINE);
        builder.setMultisampleCount(graphics.getSampleCount());
        pipelines["wireframe"] = builder.build(device, {colorFormat});
    }

    {
        // skybox pipeline
        PipelineBuilder builder;
        builder.setPipelineLayout(pipelineLayouts["skybox"]);
        builder.setShader(shaders["skybox.vert.spv"], VK_SHADER_STAGE_VERTEX_BIT);
        builder.setShader(shaders["skybox.frag.spv"], VK_SHADER_STAGE_FRAGMENT_BIT);
        builder.setCulling(VK_CULL_MODE_FRONT_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        builder.setDepthTest(VK_TRUE, VK_FALSE, VK_COMPARE_OP_EQUAL);
        builder.setMultisampleCount(graphics.getSampleCount());
        pipelines["skybox"] = builder.build(device, {colorFormat});
    }

    for (auto &[_, shader] : shaders) {
        vkDestroyShaderModule(device, shader, nullptr);
    }
}

void Renderer::destroyPipelines()
{
    ZoneScoped;

    const VkDevice device = graphics.getDevice();

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
        graphics.createImage(shadowMap, createInfo, false);

        ResourceManager::get()->addImage(shadowMap, "shadow_map");
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
        graphics.createCubemapImage(skybox, createInfo, "assets/textures/skybox");
        ResourceManager::get()->addImage(skybox, "skybox");
    }

    createBuffers();
    updateDescriptorSet();
}

void Renderer::createBuffers()
{
    ZoneScoped;

    const VkDevice device = graphics.getDevice();

    // scene data
    {
        BufferCreateInfo createInfo = {
            .size = sizeof(SceneDrawData),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        };

        graphics.createBuffer(sceneDataBuffer, createInfo);
        vulkan::setDebugName(device, reinterpret_cast<uint64_t>(sceneDataBuffer.buffer), VK_OBJECT_TYPE_BUFFER, "Scene Data buffer");
    }

    auto &materials = ResourceManager::get()->getMaterials();
    auto &lights = ResourceManager::get()->getLights();
    auto &indices = ResourceManager::get()->getIndices();
    auto &vertices = ResourceManager::get()->getVertices();

    // materials
    if (!materials.empty()) {
        BufferCreateInfo createInfo = {
            .size = materials.size() * sizeof(GPUMaterial),
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        };

        graphics.createBuffer(materialsBuffer, createInfo);
        vulkan::setDebugName(device, reinterpret_cast<uint64_t>(materialsBuffer.buffer), VK_OBJECT_TYPE_BUFFER, "Materials buffer");
        memcpy(materialsBuffer.info.pMappedData, materials.data(), materialsBuffer.size);
    }

    // lights
    if (!lights.empty()) {
        BufferCreateInfo createInfo = {
            .size = lights.size() * sizeof(GPULight),
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        };

        graphics.createBuffer(lightsBuffer, createInfo);
        vulkan::setDebugName(device, reinterpret_cast<uint64_t>(lightsBuffer.buffer), VK_OBJECT_TYPE_BUFFER, "Lights buffer");
        memcpy(lightsBuffer.info.pMappedData, lights.data(), lightsBuffer.size);
    }

    // indices
    if (!indices.empty()) {
        vulkan::BufferCreateInfo createInfo;
        createInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        createInfo.size = indices.size() * sizeof(uint32_t);

        graphics.createBuffer(indexBuffer, createInfo);
        graphics.uploadBuffer(indexBuffer, indices.data(), createInfo.size);
    }

    // vertices
    if (!vertices.empty()) {
        vulkan::BufferCreateInfo createInfo;
        createInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        createInfo.size = vertices.size() * sizeof(Vertex);

        graphics.createBuffer(vertexBuffer, createInfo);
        graphics.uploadBuffer(vertexBuffer, vertices.data(), createInfo.size);
    }
}

void Renderer::updateDescriptorSet()
{
    ZoneScoped;

    // Update descriptors, if necessary
    DescriptorWriter writer;

    auto &images = ResourceManager::get()->getImages();
    for (size_t i = 0; i < images.size(); i++) {
        writer.write(TEXTURES_BINDING, images[i].view, images[i].sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, i);
    }

    writer.write(SCENE_DATA_BINDING, sceneDataBuffer.buffer, sceneDataBuffer.size, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
    writer.write(MATERIALS_BINDING, materialsBuffer.buffer, materialsBuffer.size, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    writer.write(LIGHTS_BINDING, lightsBuffer.buffer, lightsBuffer.size, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);
    writer.write(VERTEX_BINDING, vertexBuffer.buffer, vertexBuffer.size, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0);

    writer.update(graphics.getDevice(), graphics.getDescriptorManager().getSet());
}

void Renderer::reloadShaders()
{
    ZoneScoped;

    vkDeviceWaitIdle(graphics.getDevice());
    destroyPipelines();
    createPipelines();
}

void Renderer::loadDefaultResources()
{
    //
    // Materials
    //

    // checkerboard
    {
        vulkan::Image image;
        ImageCreateInfo imageCI{};
        graphics.createImageFromFile(image, imageCI, "assets/textures/checkerboard.png");
        ResourceManager::get()->addImage(image, "checkerboard");

        GPUMaterial material;
        material.diffuseId = ResourceManager::get()->getImageIndexByName("checkerboard");
        material.metallicFactor = 0.0f;
        material.roughnessFactor = 1.0f;
        ResourceManager::get()->addMaterial(material, "checkerboard");
    }

    // colors
    {
        GPUMaterial red;
        red.color = color::red;
        red.materialFlags |= (unsigned int)MaterialFlags::Color;
        red.ambient = 1.0f;
        ResourceManager::get()->addMaterial(red, "red");

        GPUMaterial green;
        green.color = color::green;
        green.materialFlags |= (unsigned int)MaterialFlags::Color;
        green.ambient = 1.0f;
        ResourceManager::get()->addMaterial(green, "green");

        GPUMaterial blue;
        blue.color = color::blue;
        blue.materialFlags |= (unsigned int)MaterialFlags::Color;
        blue.ambient = 1.0f;
        ResourceManager::get()->addMaterial(blue, "blue");

        GPUMaterial black;
        black.color = color::black;
        black.materialFlags |= (unsigned int)MaterialFlags::Color;
        black.ambient = 1.0f;
        ResourceManager::get()->addMaterial(black, "black");

        GPUMaterial white;
        white.color = color::white;
        white.materialFlags |= (unsigned int)MaterialFlags::Color;
        white.ambient = 1.0f;
        ResourceManager::get()->addMaterial(white, "white");

        GPUMaterial yellow;
        yellow.color = color::yellow;
        yellow.materialFlags |= (unsigned int)MaterialFlags::Color;
        yellow.ambient = 1.0f;
        ResourceManager::get()->addMaterial(yellow, "yellow");

        GPUMaterial cyan;
        cyan.color = color::cyan;
        cyan.materialFlags |= (unsigned int)MaterialFlags::Color;
        cyan.ambient = 1.0f;
        ResourceManager::get()->addMaterial(cyan, "cyan");

        GPUMaterial purple;
        purple.color = color::purple;
        purple.materialFlags |= (unsigned int)MaterialFlags::Color;
        purple.ambient = 1.0f;
        ResourceManager::get()->addMaterial(purple, "purple");
    }

    // transparent
    {
        GPUMaterial whiteTransparent;
        whiteTransparent.color = color::white;
        whiteTransparent.color.a = 0.3f;
        whiteTransparent.materialFlags |= (unsigned int)MaterialFlags::Color;
        whiteTransparent.ambient = 1.0f;

        ResourceManager::get()->addMaterial(whiteTransparent, "whiteTransparent");
    }

    //
    // Models
    //

    Model primitives;
    if (!gltf::loadModel(primitives, graphics, "assets/models/primitives.glb"))
        exit(EXIT_FAILURE);

    Model gizmo;
    if (!gltf::loadModel(gizmo, graphics, "assets/models/gizmo.glb"))
        exit(EXIT_FAILURE);
}