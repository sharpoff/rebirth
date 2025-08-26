#include <rebirth/renderer.h>
#include <rebirth/util/logger.h>
#include <rebirth/vulkan/descriptor_writer.h>
#include <rebirth/vulkan/pipeline_builder.h>
#include <rebirth/vulkan/swapchain.h>
#include <rebirth/vulkan/util.h>

#include "imgui.h"

namespace rebirth
{

Renderer::Renderer(SDL_Window *window) : window(window)
{
    assert(window);

    graphics = new vulkan::Graphics(window);

    meshPipeline.initialize(*graphics);
    shadowPipeline.initialize(*graphics);
    skyboxPipeline.initialize(*graphics);
    imguiPipeline.initialize(*graphics);
}

Renderer::~Renderer()
{
    const VkDevice device = graphics->getDevice();
    vkDeviceWaitIdle(device);

    resourceManager.destroy(*graphics);

    shadowPipeline.destroy(device);
    meshPipeline.destroy(device);
    skyboxPipeline.destroy(*graphics);
    imguiPipeline.destroy(device);

    graphics->destroyBuffer(&sceneDataBuffer);
    graphics->destroyBuffer(&materialsBuffer);
    graphics->destroyBuffer(&lightsBuffer);

    delete graphics;
}

void Renderer::prepare()
{
    createResources();
    prepared = true;
}

void Renderer::render()
{
    if (!prepared) {
        util::logError("You forgot to call prepare()!");
        exit(-1);
    }

    updateDynamicData();

    const VkCommandBuffer cmd = graphics->beginCommandBuffer();

    // resizing window / recreating swapchain
    if (cmd == VK_NULL_HANDLE)
        return;

    // Skybox Pass
    {
        skyboxPipeline.beginFrame(*graphics, cmd);
        skyboxPipeline.drawSkybox(*graphics, cmd, skyboxIdx);
        skyboxPipeline.endFrame(*graphics, cmd);
    }

    // Shadow Pass
    if (meshDrawCommands.size() > 0) {
        shadowPipeline.beginFrame(*graphics, cmd);

        for (auto &light : lights)
            shadowPipeline.draw(*graphics, resourceManager, cmd, meshDrawCommands, light.mvp);

        shadowPipeline.endFrame(*graphics, cmd, debugShadows);
    }

    // Mesh Pass
    if (!debugShadows && meshDrawCommands.size() > 0) {
        meshPipeline.beginFrame(*graphics, cmd);
        meshPipeline.draw(*graphics, resourceManager, cmd, meshDrawCommands);
        meshPipeline.endFrame(*graphics, cmd);
    }

    // Imgui Pass
    {
        imguiPipeline.beginFrame(*graphics, cmd);
        updateImGui();
        imguiPipeline.endFrame(*graphics, cmd);
    }

    graphics->submitCommandBuffer(cmd);

    meshDrawCommands.clear();
    lights.clear();
}

void Renderer::addLight(Light light)
{
    if (lights.size() >= maxLightsNum) {
        util::logError("Light limit exceed");
        return;
    }

    lights.push_back(light);
}

void Renderer::addMesh(MeshIdx idx, mat4 transform, bool castShadows)
{
    meshDrawCommands.push_back(
        MeshDrawCommand{
            .meshIdx = idx,
            .transform = transform,
            .castShadows = castShadows,
        }
    );
}

void Renderer::updateImGui()
{
    ImGui::ShowDemoWindow();

    {
        auto camPos = camera->getPosition();

        ImGui::Begin("Debug");
        ImGui::Text("Camera pos: [%f, %f, %f]", camPos.x, camPos.y, camPos.z);
        ImGui::Checkbox("Debug shadows", &debugShadows);

        ImGui::End();
    }
}

void Renderer::updateDynamicData()
{
    for (auto &light : lights) {
        mat4 projection = math::perspective(glm::radians(45.0f), 1.0f, 1.0f, 100.0f);
        mat4 view = glm::lookAt(light.position, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
        mat4 mvp = projection * view;
        light.mvp = mvp;
    }
    memcpy(lightsBuffer.info.pMappedData, lights.data(), lightsBuffer.size);

    SceneData sceneData = {};
    sceneData.projection = camera->getProjection();
    sceneData.view = camera->getViewMatrix();
    sceneData.cameraPosAndLightNum = vec4(camera->getPosition(), lights.size());
    sceneData.lightsBufferAddress = lightsBuffer.address;
    sceneData.materialsBufferAddress = materialsBuffer.address;
    sceneData.shadowMapIndex = shadowMapIdx;
    memcpy(sceneDataBuffer.info.pMappedData, &sceneData, sizeof(sceneData));
}

void Renderer::updateAnimation()
{
    // if (animations.size() <= 0)
    //     return;

    // auto &animation = animations[index];
    // for (auto &channel : animation.channels) {
    //     AnimationSampler &sampler = animation.samplers[channel.sampler];

    //     for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
    //         if ((animation.currentTime >= sampler.inputs[i]) && (animation.currentTime <= sampler.inputs[i + 1])) {
    //             float step = (animation.currentTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
    //             SceneNode *node = channel.target;
    //             if (!node)
    //                 continue;

    //             if (channel.targetType == AnimationTargetType::translation) {
    //                 node->transform.setPosition(glm::mix(sampler.outputs[i], sampler.outputs[i + 1], step));
    //             }

    //             if (channel.targetType == AnimationTargetType::rotation) {
    //                 glm::quat q1;
    //                 q1.x = sampler.outputs[i].x;
    //                 q1.y = sampler.outputs[i].y;
    //                 q1.z = sampler.outputs[i].z;
    //                 q1.w = sampler.outputs[i].w;

    //                 glm::quat q2;
    //                 q2.x = sampler.outputs[i + 1].x;
    //                 q2.y = sampler.outputs[i + 1].y;
    //                 q2.z = sampler.outputs[i + 1].z;
    //                 q2.w = sampler.outputs[i + 1].w;

    //                 node->transform.setRotation(glm::normalize(glm::slerp(q1, q2, step)));
    //             }

    //             if (channel.targetType == AnimationTargetType::scale) {
    //                 node->transform.setScale(glm::mix(sampler.outputs[i], sampler.outputs[i + 1], step));
    //             }
    //         }
    //     }
    // }

    // for (auto &[_, scene] : scenes) {
    //     for (auto &node : scene.nodes) {
    //         updateJoints(node, scene);
    //     }
    // }
}

void Renderer::updateJoints(SceneNode &node, Scene &scene)
{
    // if (node.skin > -1) {
    //     glm::mat4 inverseTransform = glm::inverse(node.transform.getModelMatrix());
    //     Skin skin = scene.skins[node.skin];

    //     size_t jointsCount = (uint32_t)skin.joints.size();
    //     std::vector<glm::mat4> jointMatrices(jointsCount);

    //     for (size_t i = 0; i < jointsCount; i++) {
    //         mat4 joint = skin.joints[i]->transform.getModelMatrix();
    //         jointMatrices[i] = joint * skin.inverseBindMatrices[i];
    //         jointMatrices[i] = inverseTransform * jointMatrices[i];
    //     }

    //     // Update ssbo
    //     graphics->uploadBuffer(skin.ssbo, jointMatrices.data(), jointMatrices.size() * sizeof(glm::mat4));
    // }

    // for (auto &child : node.children) {
    //     updateJoints(child, scene);
    // }
}

void Renderer::createResources()
{
    createShadowMap();
    createSkybox();

    createBuffers();

    updateDescriptors();
}

void Renderer::createBuffers()
{
    const VkDevice device = graphics->getDevice();

    // scene data
    graphics->createBuffer(&sceneDataBuffer, sizeof(SceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    vulkan::setDebugName(device, reinterpret_cast<uint64_t>(sceneDataBuffer.buffer), VK_OBJECT_TYPE_BUFFER, "Scene Data buffer");

    // materials
    auto &materials = resourceManager.getMaterials();
    graphics->createBuffer(&materialsBuffer, maxMaterialsNum * sizeof(Material), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vulkan::setDebugName(device, reinterpret_cast<uint64_t>(materialsBuffer.buffer), VK_OBJECT_TYPE_BUFFER, "Materials buffer");
    memcpy(materialsBuffer.info.pMappedData, materials.data(), materialsBuffer.size);

    // lights
    graphics->createBuffer(&lightsBuffer, maxLightsNum * sizeof(Light), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    vulkan::setDebugName(device, reinterpret_cast<uint64_t>(lightsBuffer.buffer), VK_OBJECT_TYPE_BUFFER, "Lights buffer");
}

void Renderer::createShadowMap()
{
    vulkan::Image shadowMap;
    graphics->createImage(
        &shadowMap, shadowPipeline.getShadowMapSize(), shadowPipeline.getShadowMapSize(), VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_FORMAT_D32_SFLOAT,
        VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT
    );
    vulkan::setDebugName(graphics->getDevice(), reinterpret_cast<uint64_t>(shadowMap.image), VK_OBJECT_TYPE_IMAGE, "Shadowmap image");

    shadowMapIdx = resourceManager.addImage(shadowMap);
    shadowPipeline.setShadowMapImage(resourceManager.getImage(shadowMapIdx));
}

void Renderer::createSkybox()
{
    vulkan::Image skybox;
    graphics->createCubemapImage(&skybox, "assets/textures/skybox", VK_FORMAT_R8G8B8A8_SRGB);
    vulkan::setDebugName(graphics->getDevice(), reinterpret_cast<uint64_t>(skybox.image), VK_OBJECT_TYPE_IMAGE, "Skybox image");

    skyboxIdx = resourceManager.addImage(skybox);
}

void Renderer::updateDescriptors()
{
    auto &textures = resourceManager.getImages();

    // Update descriptors, if necessary
    DescriptorWriter writer;
    if (sceneDataBuffer.buffer != VK_NULL_HANDLE)
        writer.write(0, sceneDataBuffer.buffer, sceneDataBuffer.size, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);

    for (size_t i = 0; i < textures.size(); i++)
        writer.write(1, textures[i].view, textures[i].sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, i);

    writer.update(graphics->getDevice(), graphics->getDescriptorManager().getSet());
}

} // namespace rebirth