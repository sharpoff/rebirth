#include "graphics/gltf.h"
#include "core/material_flag.h"
#include "core/resource_manager.h"
#include "graphics/vulkan/graphics.h"

#include "util/logger.h"
#include "graphics/vulkan/graphics.h"

namespace gltf
{
    bool loadScene(vulkan::Graphics &graphics, Scene &scene, std::filesystem::path file)
    {
        cgltf_options options = {};
        cgltf_data *data = NULL;
        cgltf_result result = cgltf_parse_file(&options, file.c_str(), &data);

        if (result != cgltf_result_success) {
            LOGE("Failed to load gltf scene %s", file.c_str());
            return false;
        }

        if ((result = cgltf_load_buffers(&options, data, file.c_str())) !=
            cgltf_result_success) {
            LOGE("Failed to load buffers of gltf scene %s", file.c_str());
            return false;
        }

        if ((result = cgltf_validate(data)) != cgltf_result_success) {
            LOGE("Failed to load validate gltf scene %s", file.c_str());
            return false;
        }

        if (!data) {
            LOGE("Failed to load data for gltf scene %s", file.c_str());
            return false;
        }

        scene.name = file.stem().c_str();
        cgltf_scene *root = data->scene;

        scene.nodes.resize(root->nodes_count);
        for (size_t i = 0; i < scene.nodes.size(); i++)
            loadGltfNode(scene, scene.nodes[i], data, root->nodes[i]);

        loadGltfMaterials(data);
        loadGltfTextures(graphics, file.parent_path(), data);

        // loadGltfSkins(scene, data);
        // loadGltfAnimations(scene, data);

        cgltf_free(data);
        return true;
    }

    bool loadGltfNode(Scene &scene, SceneNode &node, cgltf_data *data, cgltf_node *gltfNode)
    {
        if (!data || !gltfNode)
            return false;

        node.name = gltfNode->name ? gltfNode->name : "Node";
        node.index = cgltf_node_index(data, gltfNode);
        node.transform = loadGltfTransform(gltfNode, false);

        if (gltfNode->skin) {
            node.skinIndex = cgltf_skin_index(data, gltfNode->skin);
        }

        if (gltfNode->mesh) {
            mat4 worldTransform = loadGltfTransform(gltfNode, true);
            loadGltfMesh(scene, worldTransform, data, gltfNode->mesh);
        }

        // recursively load child nodes
        node.children.resize(gltfNode->children_count);
        for (size_t i = 0; i < gltfNode->children_count; i++) {
            loadGltfNode(scene, node.children[i], data, gltfNode->children[i]);
            node.children[i].parentIndex = node.index;
        }

        return true;
    }

    void loadGltfMesh(Scene &scene, mat4 transform, cgltf_data *data, cgltf_mesh *gltfMesh)
    {
        Mesh mesh;
        mesh.transform = transform;

        for (size_t i = 0; i < gltfMesh->primitives_count; i++) {
            cgltf_primitive prim = gltfMesh->primitives[i];

            uint32_t materialOffset = ResourceManager::get()->getMaterialsSize();
            uint32_t vertexOffset = 0;
            uint32_t indexOffset = 0;

            uint32_t vertexCount = prim.attributes[0].data ? prim.attributes[0].data->count : 0;
            uint32_t indexCount = prim.indices ? prim.indices->count : 0;

            if (vertexCount > 0)
                vertexOffset = loadVertices(prim);
            if (indexCount > 0)
                indexOffset = loadIndices(prim);

            int materialIndex = prim.material ? materialOffset + cgltf_material_index(data, prim.material) : -1;

            Primitive primitive;
            primitive.materialIndex = materialIndex;
            primitive.indexOffset = indexOffset;
            primitive.indexCount = indexCount;
            primitive.vertexCount = vertexCount;
            primitive.vertexOffset = vertexOffset;

            mesh.primitives.push_back(primitive);
        }

        auto meshId = ResourceManager::get()->addMesh(mesh, gltfMesh->name ? gltfMesh->name : "");
        scene.meshes.push_back(meshId);
    }

    size_t loadVertices(cgltf_primitive prim)
    {
        // load vertices
        size_t vertexCount = prim.attributes[0].data->count;
        eastl::vector<float> temp(vertexCount * 4);

        eastl::vector<Vertex> vertices(vertexCount);

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

        return ResourceManager::get()->addVertices(vertices);
    }

    size_t loadIndices(cgltf_primitive prim)
    {
        eastl::vector<uint32_t> indices;
        indices.resize(prim.indices->count);
        cgltf_accessor_unpack_indices(prim.indices, indices.data(), 4, indices.size());

        return ResourceManager::get()->addIndices(indices);
    }

    void loadGltfMaterials(cgltf_data *data)
    {
        size_t textureOffset = ResourceManager::get()->getImages().size();

        for (size_t i = 0; i < data->materials_count; i++) {
            cgltf_material gltfMaterial = data->materials[i];
            GPUMaterial material{};
            material.materialFlags |= (unsigned int)(MaterialFlags::All);

            // clang-format off
            if (gltfMaterial.has_pbr_metallic_roughness) {
                if (gltfMaterial.pbr_metallic_roughness.base_color_texture.texture) {
                    material.diffuseId = textureOffset + cgltf_texture_index(data, gltfMaterial.pbr_metallic_roughness.base_color_texture.texture);
                    material.diffuseFactor = vec4(gltfMaterial.pbr_metallic_roughness.base_color_factor[0], gltfMaterial.pbr_metallic_roughness.base_color_factor[1], gltfMaterial.pbr_metallic_roughness.base_color_factor[2], gltfMaterial.pbr_metallic_roughness.base_color_factor[3]);

                    // material.materialFlags |= (unsigned int)(MaterialFlags::Diffuse);
                }

                if (gltfMaterial.pbr_metallic_roughness.metallic_roughness_texture.texture) {
                    material.metallicRoughnessId = textureOffset + cgltf_texture_index(data, gltfMaterial.pbr_metallic_roughness.metallic_roughness_texture.texture);

                    material.metallicFactor = gltfMaterial.pbr_metallic_roughness.metallic_factor;
                    material.materialFlags |= (unsigned int)(MaterialFlags::MetallicRoughness);
                }
            }

            if (gltfMaterial.normal_texture.texture) {
                material.normalId = textureOffset + cgltf_texture_index(data, gltfMaterial.normal_texture.texture);
                // material.materialFlags |= (unsigned int)(MaterialFlags::Normal);
            }

            if (gltfMaterial.emissive_texture.texture) {
                material.emissiveId = textureOffset + cgltf_texture_index(data, gltfMaterial.emissive_texture.texture);
                // material.materialFlags |= (unsigned int)(MaterialFlags::Emissive);

                // m.emissiveFactor = vec3(material.emissive_factor[0],
                // material.emissive_factor[1], material.emissive_factor[2]);
            }
            // clang-format on

            ResourceManager::get()->addMaterial(material);
        }
    }

    void loadGltfTextures(vulkan::Graphics &graphics, std::filesystem::path dir, cgltf_data *data)
    {
        for (size_t i = 0; i < data->textures_count; i++) {
            cgltf_texture gltfTexture = data->textures[i];

            vulkan::ImageCreateInfo createInfo{};
            vulkan::Image image;

            if (gltfTexture.image->uri) { // load from file
                std::filesystem::path file = dir / gltfTexture.image->uri;

                graphics.createImageFromFile(image, createInfo, file);
            } else { // load from memory
                const uint8_t *data = cgltf_buffer_view_data(gltfTexture.image->buffer_view);
                uint32_t size = gltfTexture.image->buffer_view->size;

                graphics.createImageFromMemory(image, createInfo, const_cast<unsigned char *>(data), size);
            }

            ResourceManager::get()->addImage(image, gltfTexture.name ? gltfTexture.name : "");
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
                                 : eastl::string("Animation ") + eastl::to_string(i);

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

                channel.nodeIndex = cgltf_node_index(data, gltfChannel.target_node);

                channel.samplerIndex = cgltf_animation_sampler_index(&gltfAnimation, gltfChannel.sampler);
            }

            // samplers
            animation.samplers.resize(gltfAnimation.samplers_count);
            for (size_t j = 0; j < gltfAnimation.samplers_count; j++) {
                AnimationSampler &sampler = animation.samplers[j];
                cgltf_animation_sampler gltfSampler = gltfAnimation.samplers[j];

                // inputs
                if (gltfSampler.input) {
                    sampler.inputs.resize(gltfSampler.input->count);
                    eastl::vector<float> inputs(sampler.inputs.size());
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

                    eastl::vector<float> temp(gltfSampler.output->count * 4);

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
            skin.joints.resize(gltfSkin.joints_count);
            for (size_t j = 0; j < gltfSkin.joints_count; j++) {
                skin.joints[j] = cgltf_node_index(data, gltfSkin.joints[j]);
            }

            if (gltfSkin.skeleton)
                skin.skeletonIndex = cgltf_node_index(data, gltfSkin.skeleton);

            // inverse bind matrices
            if (gltfSkin.inverse_bind_matrices) {
                assert(gltfSkin.inverse_bind_matrices->type == cgltf_type_mat4);
                eastl::vector<float> temp(gltfSkin.inverse_bind_matrices->count * 16);
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

    void loadGltfLight(GPULight &light, mat4 worldMatrix, cgltf_light *gltfLight)
    {
        if (!gltfLight) return;

        if (gltfLight->type == cgltf_light_type_directional) {
            light.type = LightType::Directional;
            light.position = math::getPosition(worldMatrix);
            light.color = glm::make_vec3(gltfLight->color);
        }
    }

    mat4 loadGltfTransform(cgltf_node *node, bool world)
    {
        if (!node)
            return mat4(1.0f);

        glm::vec3 position = vec3(0.0);
        glm::quat rotation = glm::identity<quat>();
        glm::vec3 scale = vec3(1.0);

        mat4 matrix = mat4(1.0);
        if (node->has_matrix) {
            if (world)
                cgltf_node_transform_world(node, &matrix[0][0]);
            else
                cgltf_node_transform_local(node, &matrix[0][0]);

            return matrix;
        }

        if (node->has_translation)
            position = glm::make_vec3(node->translation);

        if (node->has_scale)
            scale = glm::make_vec3(node->scale);

        if (node->has_rotation)
            rotation = glm::make_quat(node->rotation);

        return glm::translate(position) * glm::toMat4(rotation) * glm::scale(scale);
    }

} // namespace gltf
