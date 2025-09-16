#include <rebirth/graphics/gltf.h>
#include <rebirth/graphics/vulkan/graphics.h>

#include <rebirth/resource_manager.h>
#include <rebirth/util/logger.h>

namespace gltf
{
    cgltf_data *loadGltfData(std::filesystem::path file)
    {
        cgltf_options options = {};
        cgltf_data *data = NULL;
        cgltf_result result = cgltf_parse_file(&options, file.c_str(), &data);

        if (result != cgltf_result_success) {
            util::logError("Failed to load gltf scene");
            return nullptr;
        }

        if ((result = cgltf_load_buffers(&options, data, file.c_str())) !=
            cgltf_result_success) {
            util::logError("Failed to load buffers of gltf scene");
            return nullptr;
        }

        if ((result = cgltf_validate(data)) != cgltf_result_success) {
            util::logError("Failed to load validate gltf scene");
            return nullptr;
        }

        return data;
    }

    bool loadScene(Scene &scene, std::filesystem::path file)
    {
        cgltf_data *data = loadGltfData(file);
        if (!data) {
            util::logError("Failed to load scene - ", file);
            return false;
        }

        scene.name = file.stem().c_str();
        cgltf_scene *root = data->scene;
        loadGltfNodes(scene, root, data);

        loadGltfMaterials(data);
        loadGltfTextures(file.parent_path(), data);
        loadGltfSkins(scene, data);
        loadGltfAnimations(scene, data);

        cgltf_free(data);
        return true;
    }

    bool loadModel(Model &model, std::filesystem::path file)
    {
        cgltf_data *data = loadGltfData(file);
        if (!data) {
            util::logError("Failed to load mesh - ", file);
            return false;
        }

        std::function<void(cgltf_node)> loadNode = [&](cgltf_node gltfNode) {
            if (gltfNode.mesh)
                loadGltfMesh(model, data, gltfNode.mesh);

            for (size_t i = 0; i < gltfNode.children_count; i++) {
                if (gltfNode.children[i])
                    loadNode(*gltfNode.children[i]);
            }
        };

        for (size_t i = 0; i < data->nodes_count; i++) {
            loadNode(data->nodes[i]);
        }

        return true;
    }

    void loadGltfNodes(Scene &scene, cgltf_scene *gltfScene, cgltf_data *data)
    {
        if (!gltfScene || !data)
            return;

        // recursively load nodes
        scene.nodes.resize(gltfScene->nodes_count);
        for (size_t i = 0; i < scene.nodes.size(); i++)
            loadGltfNode(scene, scene.nodes[i], data, gltfScene->nodes[i]);
    }

    bool loadGltfNode(Scene &scene, SceneNode &node, cgltf_data *data, cgltf_node *gltfNode)
    {
        if (!data || !gltfNode)
            return false;

        node.name = gltfNode->name ? gltfNode->name : "Node";
        node.id = SceneNodeID(cgltf_node_index(data, gltfNode));
        loadGltfTransform(node.localTransform, gltfNode, false);

        mat4 worldMatrix = scene.getNodeWorldMatrix(&node);

        if (gltfNode->camera) {
            Camera camera;
            loadGltfCamera(camera, worldMatrix, gltfNode->camera);

            scene.cameras.push_back(camera);
        }

        if (gltfNode->light) {
            Light light;
            loadGltfLight(light, worldMatrix, gltfNode->light);

            scene.lights.push_back(g_resourceManager.addLight(light));
        }

        if (gltfNode->skin) {
            node.skinId = SkinID(cgltf_skin_index(data, gltfNode->skin));
        }

        if (gltfNode->mesh)
            loadGltfMesh(node.model, data, gltfNode->mesh);

        // recursively load child nodes
        node.children.resize(gltfNode->children_count);
        for (size_t i = 0; i < gltfNode->children_count; i++) {
            loadGltfNode(scene, node.children[i], data, gltfNode->children[i]);
            node.children[i].parentId = node.id;
        }

        return true;
    }

    bool loadGltfMesh(Model &model, cgltf_data *data, cgltf_mesh *gltfMesh)
    {
        if (!data || !gltfMesh)
            return false;

        for (size_t i = 0; i < gltfMesh->primitives_count; i++) {
            cgltf_primitive prim = gltfMesh->primitives[i];

            uint32_t materialOffset = g_resourceManager.materials.size();

            std::vector<Vertex> vertices;
            loadVertices(vertices, prim);

            std::vector<uint32_t> indices;
            loadIndices(indices, vertices.size(), prim);

            MaterialID materialId = prim.material ? MaterialID(materialOffset + cgltf_material_index(data, prim.material)) : MaterialID::Invalid;

            Mesh mesh(materialId, g_resourceManager.addVerticesAndIndices(vertices, indices), indices.size());

            MeshID meshId = g_resourceManager.addMesh(mesh);

            model.meshes.push_back(meshId);
        }

        return true;
    }

    void loadVertices(std::vector<Vertex> &vertices, cgltf_primitive prim)
    {
        // load vertices
        vertices.resize(prim.attributes[0].data->count);

        size_t vertexCount = vertices.size();
        std::vector<float> temp(vertexCount * 4);

        // position
        if (const cgltf_accessor *pos =
                cgltf_find_accessor(&prim, cgltf_attribute_type_position, 0)) {
            assert(pos->type == cgltf_type_vec3);
            cgltf_accessor_unpack_floats(pos, temp.data(), vertexCount * 3);

            for (size_t i = 0; i < vertexCount; i++) {
                vertices[i].position.x = temp[i * 3 + 0];
                vertices[i].position.y = temp[i * 3 + 1];
                vertices[i].position.z = temp[i * 3 + 2];
            }
        }

        // uv
        if (const cgltf_accessor *uv =
                cgltf_find_accessor(&prim, cgltf_attribute_type_texcoord, 0)) {
            assert(uv->type == cgltf_type_vec2);
            cgltf_accessor_unpack_floats(uv, temp.data(), vertexCount * 2);

            for (size_t i = 0; i < vertexCount; i++) {
                vertices[i].uv_x = temp[i * 2 + 0];
                vertices[i].uv_y = temp[i * 2 + 1];
            }
        }

        // normal
        if (const cgltf_accessor *normal =
                cgltf_find_accessor(&prim, cgltf_attribute_type_normal, 0)) {
            assert(normal->type == cgltf_type_vec3);
            cgltf_accessor_unpack_floats(normal, temp.data(), vertexCount * 3);

            for (size_t i = 0; i < vertexCount; i++) {
                vertices[i].normal.x = temp[i * 3 + 0];
                vertices[i].normal.y = temp[i * 3 + 1];
                vertices[i].normal.z = temp[i * 3 + 2];
            }
        }

        // tangent
        if (const cgltf_accessor *tangent =
                cgltf_find_accessor(&prim, cgltf_attribute_type_tangent, 0)) {
            assert(tangent->type == cgltf_type_vec4);
            cgltf_accessor_unpack_floats(tangent, temp.data(), vertexCount * 4);

            for (size_t i = 0; i < vertexCount; i++) {
                vertices[i].tangent.x = temp[i * 4 + 0];
                vertices[i].tangent.y = temp[i * 4 + 1];
                vertices[i].tangent.z = temp[i * 4 + 2];
                vertices[i].tangent.w = temp[i * 4 + 3];
            }
        }

        // joints
        if (const cgltf_accessor *joints =
                cgltf_find_accessor(&prim, cgltf_attribute_type_joints, 0)) {
            assert(joints->type == cgltf_type_vec4);
            cgltf_accessor_unpack_floats(joints, temp.data(), vertexCount * 4);

            for (size_t i = 0; i < vertexCount; i++) {
                vertices[i].jointIndices.x = temp[i * 4 + 0];
                vertices[i].jointIndices.y = temp[i * 4 + 1];
                vertices[i].jointIndices.z = temp[i * 4 + 2];
                vertices[i].jointIndices.w = temp[i * 4 + 3];
            }
        }

        // weights
        if (const cgltf_accessor *weights =
                cgltf_find_accessor(&prim, cgltf_attribute_type_weights, 0)) {
            assert(weights->type == cgltf_type_vec4);
            cgltf_accessor_unpack_floats(weights, temp.data(), vertexCount * 4);

            for (size_t i = 0; i < vertexCount; i++) {
                vertices[i].jointWeights.x = temp[i * 4 + 0];
                vertices[i].jointWeights.y = temp[i * 4 + 1];
                vertices[i].jointWeights.z = temp[i * 4 + 2];
                vertices[i].jointWeights.w = temp[i * 4 + 3];
            }
        }
    }

    void loadIndices(std::vector<uint32_t> &indices, size_t verticesCount, cgltf_primitive prim)
    {
        if (prim.indices) {
            indices.resize(prim.indices->count);
            cgltf_accessor_unpack_indices(prim.indices, indices.data(), 4, indices.size());
        } else {
            for (size_t i = 0; i < verticesCount; i++) {
                indices.push_back(i);
            }
        }
    }

    void loadGltfMaterials(cgltf_data *data)
    {
        size_t textureOffset = g_resourceManager.images.size();

        for (size_t i = 0; i < data->materials_count; i++) {
            cgltf_material gltfMaterial = data->materials[i];
            Material material;

            if (gltfMaterial.has_pbr_metallic_roughness) {
                if (gltfMaterial.pbr_metallic_roughness.base_color_texture.texture)
                    material.baseColorId =
                        textureOffset +
                        cgltf_texture_index(
                            data,
                            gltfMaterial.pbr_metallic_roughness.base_color_texture.texture);

                material.baseColorFactor =
                    vec4(gltfMaterial.pbr_metallic_roughness.base_color_factor[0], gltfMaterial.pbr_metallic_roughness.base_color_factor[1], gltfMaterial.pbr_metallic_roughness.base_color_factor[2], gltfMaterial.pbr_metallic_roughness.base_color_factor[3]);

                if (gltfMaterial.pbr_metallic_roughness.metallic_roughness_texture
                        .texture) {
                    material.metallicRoughnessId =
                        textureOffset +
                        cgltf_texture_index(data, gltfMaterial.pbr_metallic_roughness.metallic_roughness_texture.texture);

                    material.metallicFactor =
                        gltfMaterial.pbr_metallic_roughness.metallic_factor;
                }
            }

            if (gltfMaterial.normal_texture.texture) {
                material.normalId =
                    textureOffset +
                    cgltf_texture_index(data, gltfMaterial.normal_texture.texture);
            }

            if (gltfMaterial.emissive_texture.texture) {
                material.emissiveId =
                    textureOffset +
                    cgltf_texture_index(data, gltfMaterial.emissive_texture.texture);

                // m.emissiveFactor = vec3(material.emissive_factor[0],
                // material.emissive_factor[1], material.emissive_factor[2]);
            }

            g_resourceManager.addMaterial(material);
        }
    }

    void loadGltfTextures(std::filesystem::path dir, cgltf_data *data)
    {
        for (size_t i = 0; i < data->textures_count; i++) {
            cgltf_texture gltfTexture = data->textures[i];

            vulkan::ImageCreateInfo createInfo{};
            vulkan::Image image;

            if (gltfTexture.image->uri) { // load from file
                std::filesystem::path file = dir / gltfTexture.image->uri;

                g_graphics.createImageFromFile(image, createInfo, file);
                g_resourceManager.addImage(image);
            } else { // load from memory
                const uint8_t *data = cgltf_buffer_view_data(gltfTexture.image->buffer_view);
                uint32_t size = gltfTexture.image->buffer_view->size;

                g_graphics.createImageFromMemory(image, createInfo, const_cast<unsigned char *>(data), size);
                g_resourceManager.addImage(image);
            }
        }
    }

    void loadGltfAnimations(Scene &scene, cgltf_data *data)
    {
        scene.animations.resize(data->animations_count);
        for (size_t i = 0; i < data->animations_count; i++) {
            Animation &animation = scene.animations[i];
            cgltf_animation gltfAnimation = data->animations[i];

            animation.name = gltfAnimation.name
                                 ? gltfAnimation.name
                                 : std::string("Animation ") + std::to_string(i);

            // channels
            animation.channels.resize(gltfAnimation.channels_count);
            for (size_t j = 0; j < gltfAnimation.channels_count; j++) {
                AnimationChannel &channel = animation.channels[j];
                cgltf_animation_channel gltfChannel = gltfAnimation.channels[j];

                switch (gltfChannel.target_path) {
                    case cgltf_animation_path_type_translation:
                        channel.path = AnimationPath::translation;
                        break;
                    case cgltf_animation_path_type_rotation:
                        channel.path = AnimationPath::rotation;
                        break;
                    case cgltf_animation_path_type_scale:
                        channel.path = AnimationPath::scale;
                        break;
                    case cgltf_animation_path_type_weights:
                        channel.path = AnimationPath::weights;
                        break;
                    default:
                        channel.path = AnimationPath::invalid;
                        break;
                }

                channel.nodeId = SceneNodeID(cgltf_node_index(data, gltfChannel.target_node));

                channel.samplerId = SamplerID(cgltf_animation_sampler_index(&gltfAnimation, gltfChannel.sampler));
            }

            // samplers
            animation.samplers.resize(gltfAnimation.samplers_count);
            for (size_t j = 0; j < gltfAnimation.samplers_count; j++) {
                AnimationSampler &sampler = animation.samplers[j];
                cgltf_animation_sampler gltfSampler = gltfAnimation.samplers[j];

                // inputs
                if (gltfSampler.input) {
                    sampler.inputs.resize(gltfSampler.input->count);
                    std::vector<float> inputs(sampler.inputs.size());
                    cgltf_accessor_unpack_floats(gltfSampler.input, inputs.data(), inputs.size());

                    for (size_t k = 0; k < sampler.inputs.size(); k++) {
                        sampler.inputs[k] = inputs[k];

                        // figure out start and end of animation
                        if (inputs[k] < animation.start)
                            animation.start = inputs[k];

                        if (inputs[k] > animation.end)
                            animation.end = inputs[k];
                    }
                }

                // outputs
                if (gltfSampler.output) {
                    sampler.outputs.resize(gltfSampler.output->count);

                    std::vector<float> temp(gltfSampler.output->count * 4);

                    for (size_t k = 0; k < sampler.outputs.size(); k++) {
                        if (gltfSampler.output->type == cgltf_type_vec4) {
                            cgltf_accessor_unpack_floats(gltfSampler.output, temp.data(), gltfSampler.output->count * 4);
                            sampler.outputs[k].x = temp[k * 4 + 0];
                            sampler.outputs[k].y = temp[k * 4 + 1];
                            sampler.outputs[k].z = temp[k * 4 + 2];
                            sampler.outputs[k].w = temp[k * 4 + 3];
                        } else if (gltfSampler.output->type == cgltf_type_vec3) {
                            cgltf_accessor_unpack_floats(gltfSampler.output, temp.data(), gltfSampler.output->count * 3);
                            sampler.outputs[k].x = temp[k * 3 + 0];
                            sampler.outputs[k].y = temp[k * 3 + 1];
                            sampler.outputs[k].z = temp[k * 3 + 2];
                            sampler.outputs[k].w = 0.0f;
                        }
                    }
                }
            }
        }
    }

    void loadGltfSkins(Scene &scene, cgltf_data *data)
    {
        scene.skins.resize(data->skins_count);
        for (size_t i = 0; i < data->skins_count; i++) {
            Skin &skin = scene.skins[i];
            cgltf_skin gltfSkin = data->skins[i];

            skin.name = gltfSkin.name ? gltfSkin.name : "skin";

            // joints
            skin.jointIds.resize(gltfSkin.joints_count);
            for (size_t j = 0; j < gltfSkin.joints_count; j++) {
                skin.jointIds[j] = SceneNodeID(cgltf_node_index(data, gltfSkin.joints[j]));
            }

            if (gltfSkin.skeleton)
                skin.skeletonId = SceneNodeID(cgltf_node_index(data, gltfSkin.skeleton));

            // inverse bind matrices
            if (gltfSkin.inverse_bind_matrices) {
                assert(gltfSkin.inverse_bind_matrices->type == cgltf_type_mat4);
                std::vector<float> temp(gltfSkin.inverse_bind_matrices->count * 16);
                cgltf_accessor_unpack_floats(gltfSkin.inverse_bind_matrices, temp.data(), temp.size());

                skin.inverseBindMatrices.resize(gltfSkin.inverse_bind_matrices->count);
                for (size_t j = 0; j < skin.inverseBindMatrices.size(); j++) {
                    skin.inverseBindMatrices[j] = {
                        temp[j * 16 + 0],
                        temp[j * 16 + 1],
                        temp[j * 16 + 2],
                        temp[j * 16 + 3],
                        temp[j * 16 + 4],
                        temp[j * 16 + 5],
                        temp[j * 16 + 6],
                        temp[j * 16 + 7],
                        temp[j * 16 + 8],
                        temp[j * 16 + 9],
                        temp[j * 16 + 10],
                        temp[j * 16 + 11],
                        temp[j * 16 + 12],
                        temp[j * 16 + 13],
                        temp[j * 16 + 14],
                        temp[j * 16 + 15],
                    };
                }

                // TODO: this should be changed to global joint materices buffer
                // vulkan::BufferCreateInfo createInfo = {
                //     .size = sizeof(mat4) * skin.inverseBindMatrices.size(),
                //     .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                // };
                // g_graphics.createBuffer(skin.jointMatricesBuffer, createInfo);
            }
        }
    }

    bool loadGltfCamera(Camera &camera, mat4 worldMatrix, cgltf_camera *gltfCamera)
    {
        if (!gltfCamera)
            return false;

        // TODO: add support for orthographic
        if (gltfCamera->type == cgltf_camera_type_perspective) {
            cgltf_camera_perspective gltfPerspective = gltfCamera->data.perspective;

            auto extent = g_graphics.getSwapchain().getExtent();

            float near = 0.1f;
            float far = 300.0f;
            if (gltfPerspective.has_zfar) {
                far = gltfPerspective.zfar;
            }

            camera.setPerspective(gltfPerspective.yfov, float(extent.width) / extent.height, near, far);
            camera.setPosition(math::getPosition(worldMatrix));
        }

        return true;
    }

    bool loadGltfLight(Light &light, mat4 worldMatrix, cgltf_light *gltfLight)
    {
        if (!gltfLight)
            return false;

        if (gltfLight->type == cgltf_light_type_directional) {
            light.type = LightType::Directional;
            light.position = math::getPosition(worldMatrix);
            light.color = glm::make_vec3(gltfLight->color);
        }

        return true;
    }

    bool loadGltfTransform(Transform &transform, cgltf_node *node, bool world)
    {
        if (!node) {
            transform = Transform();
            return false;
        }

        glm::vec3 position = vec3(0.0);
        glm::quat rotation = glm::identity<quat>();
        glm::vec3 scale = vec3(1.0);

        mat4 matrix = mat4(1.0);
        if (node->has_matrix) {
            if (world)
                cgltf_node_transform_world(node, &matrix[0][0]);
            else
                cgltf_node_transform_local(node, &matrix[0][0]);
        }

        if (node->has_translation)
            position = glm::make_vec3(node->translation);

        if (node->has_scale)
            scale = glm::make_vec3(node->scale);

        if (node->has_rotation)
            rotation = glm::make_quat(node->rotation);

        matrix = glm::translate(mat4(1.0f), position) * mat4(rotation) *
                 glm::scale(mat4(1.0f), scale) * matrix;

        transform = Transform(matrix);
        return true;
    }

} // namespace gltf