#include <rebirth/graphics/gltf.h>
#include <rebirth/graphics/vulkan/graphics.h>

#include <rebirth/util/logger.h>
#include <rebirth/graphics/renderer.h>

namespace gltf
{
    bool loadScene(Renderer &renderer, Scene &scene, std::filesystem::path file)
    {
        cgltf_options options = {};
        cgltf_data *data = NULL;
        cgltf_result result = cgltf_parse_file(&options, file.c_str(), &data);

        if (result != cgltf_result_success) {
            logger::logError("Failed to load gltf scene");
            return false;
        }

        if ((result = cgltf_load_buffers(&options, data, file.c_str())) !=
            cgltf_result_success) {
            logger::logError("Failed to load buffers of gltf scene");
            return false;
        }

        if ((result = cgltf_validate(data)) != cgltf_result_success) {
            logger::logError("Failed to load validate gltf scene");
            return false;
        }

        if (!data) {
            logger::logError("Failed to load scene - ", file);
            return false;
        }

        scene.name = file.stem().c_str();
        cgltf_scene *root = data->scene;
        if (!root) {
            logger::logError("Failed to load scene - root is NULL!");
            return false;
        }

        scene.nodes.resize(root->nodes_count);
        for (size_t i = 0; i < scene.nodes.size(); i++)
            loadGltfNode(renderer, scene, scene.nodes[i], data, root->nodes[i]);

        loadGltfMaterials(renderer, data);
        loadGltfTextures(renderer, file.parent_path(), data);

        // loadGltfSkins(scene, data);
        // loadGltfAnimations(scene, data);

        cgltf_free(data);
        return true;
    }

    bool loadGltfNode(Renderer &renderer, Scene &scene, SceneNode &node, cgltf_data *data, cgltf_node *gltfNode)
    {
        if (!data || !gltfNode)
            return false;

        node.name = gltfNode->name ? gltfNode->name : "Node";
        node.index = cgltf_node_index(data, gltfNode);
        loadGltfTransform(node.transform, gltfNode, false);

        if (gltfNode->skin) {
            node.skinIndex = cgltf_skin_index(data, gltfNode->skin);
        }

        if (gltfNode->mesh)
            loadGltfMesh(renderer, scene, node.mesh, data, gltfNode->mesh);

        // recursively load child nodes
        node.children.resize(gltfNode->children_count);
        for (size_t i = 0; i < gltfNode->children_count; i++) {
            loadGltfNode(renderer, scene, node.children[i], data, gltfNode->children[i]);
            node.children[i].parentIndex = node.index;
        }

        return true;
    }

    bool loadGltfMesh(Renderer &renderer, Scene &scene, Mesh &mesh, cgltf_data *data, cgltf_mesh *gltfMesh)
    {
        if (!data || !gltfMesh)
            return false;

        for (size_t i = 0; i < gltfMesh->primitives_count; i++) {
            cgltf_primitive prim = gltfMesh->primitives[i];

            uint32_t materialOffset = renderer.materials.size();
            uint32_t vertexOffset = renderer.vertices.size();
            uint32_t indexOffset = renderer.indices.size();

            uint32_t vertexCount = loadVertices(renderer.vertices, prim);

            uint32_t indexCount = loadIndices(renderer.indices, prim);

            int materialIndex = prim.material ? materialOffset + cgltf_material_index(data, prim.material) : -1;

            Primitive primitive;
            primitive.materialIndex = materialIndex;
            primitive.indexOffset = indexOffset;
            primitive.indexCount = indexCount;
            primitive.vertexCount = vertexCount;
            primitive.vertexOffset = vertexOffset;

            mesh.primitives.push_back(primitive);
        }

        return true;
    }

    // return new vertices count
    size_t loadVertices(std::vector<Vertex> &vertices, cgltf_primitive prim)
    {
        // load vertices
        size_t vertexCount = prim.attributes[0].data->count;
        std::vector<float> temp(vertexCount * 4);

        std::vector<Vertex> newVertices(vertexCount);

        // position
        if (const cgltf_accessor *pos =
                cgltf_find_accessor(&prim, cgltf_attribute_type_position, 0)) {
            assert(pos->type == cgltf_type_vec3);
            cgltf_accessor_unpack_floats(pos, temp.data(), vertexCount * 3);

            for (size_t i = 0; i < vertexCount; i++) {
                newVertices[i].position.x = temp[i * 3 + 0];
                newVertices[i].position.y = temp[i * 3 + 1];
                newVertices[i].position.z = temp[i * 3 + 2];
            }
        }

        // uv
        if (const cgltf_accessor *uv =
                cgltf_find_accessor(&prim, cgltf_attribute_type_texcoord, 0)) {
            assert(uv->type == cgltf_type_vec2);
            cgltf_accessor_unpack_floats(uv, temp.data(), vertexCount * 2);

            for (size_t i = 0; i < vertexCount; i++) {
                newVertices[i].uv_x = temp[i * 2 + 0];
                newVertices[i].uv_y = temp[i * 2 + 1];
            }
        }

        // normal
        if (const cgltf_accessor *normal =
                cgltf_find_accessor(&prim, cgltf_attribute_type_normal, 0)) {
            assert(normal->type == cgltf_type_vec3);
            cgltf_accessor_unpack_floats(normal, temp.data(), vertexCount * 3);

            for (size_t i = 0; i < vertexCount; i++) {
                newVertices[i].normal.x = temp[i * 3 + 0];
                newVertices[i].normal.y = temp[i * 3 + 1];
                newVertices[i].normal.z = temp[i * 3 + 2];
            }
        }

        // tangent
        if (const cgltf_accessor *tangent =
                cgltf_find_accessor(&prim, cgltf_attribute_type_tangent, 0)) {
            assert(tangent->type == cgltf_type_vec4);
            cgltf_accessor_unpack_floats(tangent, temp.data(), vertexCount * 4);

            for (size_t i = 0; i < vertexCount; i++) {
                newVertices[i].tangent.x = temp[i * 4 + 0];
                newVertices[i].tangent.y = temp[i * 4 + 1];
                newVertices[i].tangent.z = temp[i * 4 + 2];
                newVertices[i].tangent.w = temp[i * 4 + 3];
            }
        }

        // joints
        if (const cgltf_accessor *joints =
                cgltf_find_accessor(&prim, cgltf_attribute_type_joints, 0)) {
            assert(joints->type == cgltf_type_vec4);
            cgltf_accessor_unpack_floats(joints, temp.data(), vertexCount * 4);

            for (size_t i = 0; i < vertexCount; i++) {
                newVertices[i].jointIndices.x = temp[i * 4 + 0];
                newVertices[i].jointIndices.y = temp[i * 4 + 1];
                newVertices[i].jointIndices.z = temp[i * 4 + 2];
                newVertices[i].jointIndices.w = temp[i * 4 + 3];
            }
        }

        // weights
        if (const cgltf_accessor *weights =
                cgltf_find_accessor(&prim, cgltf_attribute_type_weights, 0)) {
            assert(weights->type == cgltf_type_vec4);
            cgltf_accessor_unpack_floats(weights, temp.data(), vertexCount * 4);

            for (size_t i = 0; i < vertexCount; i++) {
                newVertices[i].jointWeights.x = temp[i * 4 + 0];
                newVertices[i].jointWeights.y = temp[i * 4 + 1];
                newVertices[i].jointWeights.z = temp[i * 4 + 2];
                newVertices[i].jointWeights.w = temp[i * 4 + 3];
            }
        }

        vertices.insert(vertices.end(), newVertices.begin(), newVertices.end());

        return vertexCount;
    }

    // return new indices count
    size_t loadIndices(std::vector<uint32_t> &indices, cgltf_primitive prim)
    {
        std::vector<uint32_t> newIndices;
        if (prim.indices) {
            newIndices.resize(prim.indices->count);
            cgltf_accessor_unpack_indices(prim.indices, newIndices.data(), 4, newIndices.size());
        } else {
            logger::logWarn("Failed to loadIndices(), no indices!");
        }

        indices.insert(indices.begin(), newIndices.begin(), newIndices.end());

        return newIndices.size();
    }

    void loadGltfMaterials(Renderer &renderer, cgltf_data *data)
    {
        size_t textureOffset = renderer.images.size();

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

            renderer.materials.push_back(material);
        }
    }

    void loadGltfTextures(Renderer &renderer, std::filesystem::path dir, cgltf_data *data)
    {
        for (size_t i = 0; i < data->textures_count; i++) {
            cgltf_texture gltfTexture = data->textures[i];

            vulkan::ImageCreateInfo createInfo{};
            vulkan::Image image;

            if (gltfTexture.image->uri) { // load from file
                std::filesystem::path file = dir / gltfTexture.image->uri;

                renderer.getGraphics().createImageFromFile(image, createInfo, file);
            } else { // load from memory
                const uint8_t *data = cgltf_buffer_view_data(gltfTexture.image->buffer_view);
                uint32_t size = gltfTexture.image->buffer_view->size;

                renderer.getGraphics().createImageFromMemory(image, createInfo, const_cast<unsigned char *>(data), size);
            }

            renderer.images.push_back(image);
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
            skin.joints.resize(gltfSkin.joints_count);
            for (size_t j = 0; j < gltfSkin.joints_count; j++) {
                skin.joints[j] = cgltf_node_index(data, gltfSkin.joints[j]);
            }

            if (gltfSkin.skeleton)
                skin.skeletonIndex = cgltf_node_index(data, gltfSkin.skeleton);

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

    bool loadGltfTransform(mat4 transform, cgltf_node *node, bool world)
    {
        if (!node)
            return false;

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

        transform = glm::translate(mat4(1.0f), position) * mat4(rotation) *
                    glm::scale(mat4(1.0f), scale) * matrix;

        return true;
    }

} // namespace gltf