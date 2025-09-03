#include <rebirth/renderer.h>
#include <rebirth/types/scene.h>
#include <rebirth/util/logger.h>
#include <rebirth/vulkan/descriptor_writer.h>
#include <rebirth/vulkan/pipeline_builder.h>
#include <rebirth/vulkan/swapchain.h>
#include <rebirth/vulkan/util.h>

#include "imgui.h"

namespace rebirth
{

void Renderer::initialize(SDL_Window *window)
{
    assert(window);
    this->window = window;

    graphics.initialize(window);

    // pipelines
    meshPipeline.initialize(graphics);
    shadowPipeline.initialize(graphics);
    skyboxPipeline.initialize(graphics);
    imguiPipeline.initialize(graphics);
    wireframePipeline.initialize(graphics);

    Image checkerboard;
    graphics.createImageFromFile(&checkerboard, "assets/textures/checkerboard.png");
    defaultImageId = resourceManager.addImage(checkerboard);

    Material defaultMaterial;
    defaultMaterial.baseColorIdx = defaultImageId;
    defaultMaterialId = resourceManager.addMaterial(defaultMaterial);
}

void Renderer::shutdown()
{
    const VkDevice device = graphics.getDevice();
    vkDeviceWaitIdle(device);

    resourceManager.destroy(graphics);

    shadowPipeline.destroy(device);
    meshPipeline.destroy(device);
    skyboxPipeline.destroy(graphics);
    imguiPipeline.destroy(device);
    wireframePipeline.destroy(device);

    graphics.destroyBuffer(&sceneDataBuffer);
    graphics.destroyBuffer(&materialsBuffer);
    graphics.destroyBuffer(&lightsBuffer);

    graphics.destroy();
}

void Renderer::addLight(Light light) { resourceManager.addLight(light); }

void Renderer::drawScene(Scene &scene, Transform transform)
{
    std::function<void(SceneNode &)> nodeDraw = [&](SceneNode &node) {
        for (auto &mesh : node.mesh.primitives) {
            drawCommands.push_back(
                DrawCommand{
                    .meshId = mesh,
                    .transform = transform.getModelMatrix() * scene.getNodeWorldMatrix(&node),
                    .boundingSphere = Sphere(),
                    .jointMatricesBuffer =
                        node.skin > -1 ? scene.skins[node.skin].jointMatricesBuffer.address : 0,
                }
            );
        }

        for (auto &child : node.children) {
            nodeDraw(child);
        }
    };

    for (auto &node : scene.nodes) {
        nodeDraw(node);
    }
}

void Renderer::drawObject(Object object)
{
    drawScene(object.scene, object.transform * object.scene.transform);
}

void Renderer::present(ApplicationState &state, Camera &camera)
{
    if (!prepared) {
        createResources();
        prepared = true;
    }

    updateDynamicData(camera);

    Frustum frustum(camera);

    const VkCommandBuffer cmd = graphics.beginCommandBuffer();

    // resizing window / recreating swapchain
    if (cmd == VK_NULL_HANDLE)
        return;

    // Skybox Pass
    {
        skyboxPipeline.beginFrame(graphics, cmd);
        if (state.skybox) {
            skyboxPipeline.drawSkybox(graphics, cmd, skyboxIdx);
        }
        skyboxPipeline.endFrame(graphics, cmd);
    }

    // Shadow Pass
    if (!drawCommands.empty()) {
        Image &shadowMap = resourceManager.getImage(shadowMapIdx);

        shadowPipeline.beginFrame(graphics, cmd, shadowMap);

        if (state.shadows) {
            auto &lights = resourceManager.lights;
            for (auto &light : lights)
                shadowPipeline.draw(graphics, resourceManager, cmd, drawCommands, light.mvp);
        }

        shadowPipeline.endFrame(graphics, cmd, shadowMap, state.debugShadowMap);
    }

    // Mesh Pass
    if (!state.debugShadowMap && !drawCommands.empty()) {
        if (!state.wireframe) {
            meshPipeline.beginFrame(graphics, cmd);
            meshPipeline.draw(graphics, resourceManager, cmd, frustum, drawCommands);
            meshPipeline.endFrame(graphics, cmd);
        } else {
            wireframePipeline.beginFrame(graphics, cmd);
            wireframePipeline.draw(
                graphics, resourceManager, cmd, frustum, drawCommands, vec4(0.0, 1.0, 0.0, 1.0)
            );
            wireframePipeline.endFrame(graphics, cmd);
        }
    }

    // Imgui Pass
    {
        imguiPipeline.beginFrame(graphics, cmd);
        if (state.imgui) {
            updateImGui(state, camera);
        }
        imguiPipeline.endFrame(graphics, cmd);
    }

    graphics.submitCommandBuffer(cmd);

    drawCommands.clear();
}

void Renderer::updateDynamicData(Camera &camera)
{
    auto &lights = resourceManager.lights;
    for (auto &light : lights) {
        mat4 projection = math::perspective(glm::radians(45.0f), 1.0f, 1.0f, 100.0f);
        mat4 view = glm::lookAt(light.position, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
        mat4 mvp = projection * view;
        light.mvp = mvp;
    }
    memcpy(lightsBuffer.info.pMappedData, lights.data(), lightsBuffer.size);

    SceneDrawData sceneData = {};
    sceneData.projection = camera.projection;
    sceneData.view = camera.view;
    sceneData.cameraPosAndLightNum = vec4(camera.position, lights.size());
    sceneData.shadowMapIndex = shadowMapIdx;
    memcpy(sceneDataBuffer.info.pMappedData, &sceneData, sizeof(sceneData));
}

void Renderer::updateImGui(ApplicationState &state, Camera &camera)
{
    ImGui::ShowDemoWindow();

    {
        auto camPos = camera.position;

        ImGui::Begin("Debug");

        ImGui::Text("Camera pos: [%f, %f, %f]", camPos.x, camPos.y, camPos.z);
        ImGui::Checkbox("Debug shadow map", &state.debugShadowMap);
        ImGui::Checkbox("Enable wireframe", &state.wireframe);
        ImGui::Checkbox("Enable shadows", &state.shadows);
        ImGui::Checkbox("Enable skybox", &state.skybox);

        {
            ImGui::Checkbox("Enable imgui", &state.imgui);

            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("You can toggle it using 'H' key.");
            }
        }

        ImGui::End();
    }

    {
        ImGui::Begin("Lights");

        auto &lights = resourceManager.lights;
        for (size_t i = 0; i < lights.size(); i++) {
            if (ImGui::TreeNode(std::string("Light " + std::to_string(i)).c_str())) {
                ImGui::DragFloat3("position", &lights[i].position[0], 1.0f, -100.0f, 100.0f);

                ImGui::TreePop();
            }
        }
        ImGui::End();
    }

    // {
    //     ImGui::Begin("Scenes");

    //     auto &scenes = state.scenes;
    //     for (size_t i = 0; i < scenes.size(); i++) {
    //         std::vector<std::string> animationNames;
    //         for (auto &anim : scenes[i].animations)
    //             animationNames.push_back(anim.name);

    //         if (ImGui::TreeNode(std::string("Scene " + std::to_string(i)).c_str())) {
    //             if (ImGui::BeginCombo("Animations", scenes[i].currentAnimation.c_str())) {
    //                 for (size_t n = 0; n < animationNames.size(); n++) {
    //                     const bool is_selected = scenes[i].currentAnimation == animationNames[n];
    //                     if (ImGui::Selectable(animationNames[n].c_str(), is_selected))
    //                         scenes[i].currentAnimation = animationNames[n];

    //                     // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
    //                     if (is_selected)
    //                         ImGui::SetItemDefaultFocus();
    //                 }
    //                 ImGui::EndCombo();
    //             }

    //             ImGui::TreePop();
    //         }
    //     }
    //     ImGui::End();
    // }
}

void Renderer::createResources()
{
    // shadow map
    vulkan::Image shadowMap;
    graphics.createImage(
        &shadowMap, shadowPipeline.getShadowMapSize(), shadowPipeline.getShadowMapSize(),
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_FORMAT_D32_SFLOAT, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT
    );
    vulkan::setDebugName(
        graphics.getDevice(), reinterpret_cast<uint64_t>(shadowMap.image), VK_OBJECT_TYPE_IMAGE,
        "Shadowmap image"
    );

    shadowMapIdx = resourceManager.addImage(shadowMap);

    // skybox
    vulkan::Image skybox;
    graphics.createCubemapImage(&skybox, "assets/textures/skybox", VK_FORMAT_R8G8B8A8_SRGB);
    vulkan::setDebugName(
        graphics.getDevice(), reinterpret_cast<uint64_t>(skybox.image), VK_OBJECT_TYPE_IMAGE,
        "Skybox image"
    );
    skyboxIdx = resourceManager.addImage(skybox);

    createBuffers();

    updateDescriptors();
}

void Renderer::createBuffers()
{
    const VkDevice device = graphics.getDevice();

    // scene data
    graphics.createBuffer(
        &sceneDataBuffer, sizeof(SceneDrawData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
    );
    vulkan::setDebugName(
        device, reinterpret_cast<uint64_t>(sceneDataBuffer.buffer), VK_OBJECT_TYPE_BUFFER,
        "Scene Data buffer"
    );

    // materials
    auto &materials = resourceManager.materials;
    graphics.createBuffer(
        &materialsBuffer, materials.size() * sizeof(Material),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT
    );
    vulkan::setDebugName(
        device, reinterpret_cast<uint64_t>(materialsBuffer.buffer), VK_OBJECT_TYPE_BUFFER,
        "Materials buffer"
    );
    memcpy(materialsBuffer.info.pMappedData, materials.data(), materialsBuffer.size);

    // lights
    auto &lights = resourceManager.lights;
    graphics.createBuffer(
        &lightsBuffer, lights.size() * sizeof(Light),
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT
    );
    vulkan::setDebugName(
        device, reinterpret_cast<uint64_t>(lightsBuffer.buffer), VK_OBJECT_TYPE_BUFFER,
        "Lights buffer"
    );
    memcpy(lightsBuffer.info.pMappedData, lights.data(), lightsBuffer.size);
}

void Renderer::updateDescriptors()
{
    // Update descriptors, if necessary
    DescriptorWriter writer;
    if (sceneDataBuffer.buffer != VK_NULL_HANDLE) {
        writer.write(
            SCENE_DATA_BINDING, sceneDataBuffer.buffer, sceneDataBuffer.size,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0
        );
    }

    auto &textures = resourceManager.images;
    for (size_t i = 0; i < textures.size(); i++) {
        writer.write(
            TEXTURES_BINDING, textures[i].view, textures[i].sampler,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, i
        );
    }

    auto &materials = resourceManager.materials;
    for (size_t i = 0; i < materials.size(); i++) {
        writer.write(
            MATERIALS_BINDING, materialsBuffer.buffer, materialsBuffer.size,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, i
        );
    }

    auto &lights = resourceManager.lights;
    for (size_t i = 0; i < lights.size(); i++) {
        writer.write(
            LIGHTS_BINDING, lightsBuffer.buffer, lightsBuffer.size,
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, i
        );
    }

    writer.update(graphics.getDevice(), graphics.getDescriptorManager().getSet());
}

} // namespace rebirth
