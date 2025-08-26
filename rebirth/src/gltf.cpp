#include <rebirth/gltf.h>
#include <rebirth/resource_manager.h>
#include <rebirth/vulkan/graphics.h>

#include <glm/gtx/matrix_decompose.hpp>

namespace rebirth::gltf
{

bool loadScene(Scene *scene, vulkan::Graphics &graphics, ResourceManager &resourceManager, std::filesystem::path file)
{
    if (!scene)
        return false;

    cgltf_options options = {};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, file.c_str(), &data);
    if (result != cgltf_result_success) {
        printf("Failed to load gltf scene.\n");
        return false;
    }

    result = cgltf_load_buffers(&options, data, file.c_str());

    if (result != cgltf_result_success) {
        printf("Failed to load gltf scene.\n");
        return false;
    }

    scene->name = file.stem();
    size_t materialOffset = resourceManager.getMaterialsCount();

    cgltf_scene *root = data->scene;

    // recursively load nodes
    if (root) {
        scene->nodes.resize(root->nodes_count);
        for (size_t i = 0; i < scene->nodes.size(); i++)
            loadNode(graphics, resourceManager, *scene, scene->nodes[i], data, root->nodes[i], materialOffset);
    }

    loadMaterials(resourceManager, data);
    loadTextures(graphics, resourceManager, file.parent_path(), data);
    // loadAnimations(scene, data);
    // loadSkins(scene, data);

    cgltf_free(data);
    return true;
}

void loadNode(vulkan::Graphics &graphics, ResourceManager &resourceManager, Scene &scene, SceneNode &node, cgltf_data *data, cgltf_node *gltfNode, uint32_t materialOffset)
{
    if (!data || !gltfNode)
        return;

    node.index = cgltf_node_index(data, gltfNode);
    node.transform = loadTransform(gltfNode);

    if (gltfNode->skin)
        node.skin = cgltf_skin_index(data, gltfNode->skin);

    if (gltfNode->mesh)
        loadMesh(graphics, resourceManager, node, data, gltfNode->mesh, materialOffset);

    // recursively load child nodes
    node.children.resize(gltfNode->children_count);
    for (size_t i = 0; i < gltfNode->children_count; i++) {
        loadNode(graphics, resourceManager, scene, node.children[i], data, gltfNode->children[i], materialOffset);
        node.children[i].parent = &node;
    }
}

void loadMesh(vulkan::Graphics &graphics, ResourceManager &resourceManager, SceneNode &node, cgltf_data *data, cgltf_mesh *gltfMesh, uint32_t materialOffset)
{
    if (!data || !gltfMesh)
        return;

    for (size_t i = 0; i < gltfMesh->primitives_count; i++) {
        cgltf_primitive prim = gltfMesh->primitives[i];

        // load vertices
        std::vector<Vertex> vertices(prim.attributes[0].data->count);
        size_t vertexCount = vertices.size();
        std::vector<float> temp(vertexCount * 4);

        // position
        if (const cgltf_accessor *pos = cgltf_find_accessor(&prim, cgltf_attribute_type_position, 0)) {
            assert(pos->type == cgltf_type_vec3);
            cgltf_accessor_unpack_floats(pos, temp.data(), vertexCount * 3);

            for (size_t i = 0; i < vertexCount; i++) {
                vertices[i].position.x = temp[i * 3 + 0];
                vertices[i].position.y = temp[i * 3 + 1];
                vertices[i].position.z = temp[i * 3 + 2];
            }
        }

        // uv
        if (const cgltf_accessor *uv = cgltf_find_accessor(&prim, cgltf_attribute_type_texcoord, 0)) {
            assert(uv->type == cgltf_type_vec2);
            cgltf_accessor_unpack_floats(uv, temp.data(), vertexCount * 2);

            for (size_t i = 0; i < vertexCount; i++) {
                vertices[i].uv_x = temp[i * 2 + 0];
                vertices[i].uv_y = temp[i * 2 + 1];
            }
        }

        // normal
        if (const cgltf_accessor *normal = cgltf_find_accessor(&prim, cgltf_attribute_type_normal, 0)) {
            assert(normal->type == cgltf_type_vec3);
            cgltf_accessor_unpack_floats(normal, temp.data(), vertexCount * 3);

            for (size_t i = 0; i < vertexCount; i++) {
                vertices[i].normal.x = temp[i * 3 + 0];
                vertices[i].normal.y = temp[i * 3 + 1];
                vertices[i].normal.z = temp[i * 3 + 2];
            }
        }

        // tangent
        if (const cgltf_accessor *tangent = cgltf_find_accessor(&prim, cgltf_attribute_type_tangent, 0)) {
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
        if (const cgltf_accessor *joints = cgltf_find_accessor(&prim, cgltf_attribute_type_joints, 0)) {
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
        if (const cgltf_accessor *weights = cgltf_find_accessor(&prim, cgltf_attribute_type_weights, 0)) {
            assert(weights->type == cgltf_type_vec4);
            cgltf_accessor_unpack_floats(weights, temp.data(), vertexCount * 4);

            for (size_t i = 0; i < vertexCount; i++) {
                vertices[i].jointWeights.x = temp[i * 4 + 0];
                vertices[i].jointWeights.y = temp[i * 4 + 1];
                vertices[i].jointWeights.z = temp[i * 4 + 2];
                vertices[i].jointWeights.w = temp[i * 4 + 3];
            }
        }

        // load indices
        std::vector<uint32_t> indices(prim.indices->count);
        cgltf_accessor_unpack_indices(prim.indices, indices.data(), 4, indices.size());

        GPUMesh mesh;
        mesh.indexCount = indices.size();
        mesh.vertexCount = vertices.size();
        mesh.materialIdx = materialOffset + cgltf_material_index(data, prim.material);

        graphics.createBuffer(&mesh.vertexBuffer, vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        graphics.createBuffer(&mesh.indexBuffer, indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        graphics.uploadBuffer(mesh.vertexBuffer, vertices.data(), vertices.size() * sizeof(Vertex));
        graphics.uploadBuffer(mesh.indexBuffer, indices.data(), indices.size() * sizeof(uint32_t));

        size_t meshIdx = resourceManager.addMesh(mesh);

        node.mesh.primitives.push_back(meshIdx);
    }
}

void loadMaterials(ResourceManager &resourceManager, cgltf_data *data)
{
    size_t textureOffset = resourceManager.getImagesCount();

    for (size_t i = 0; i < data->materials_count; i++) {
        cgltf_material gltfMaterial = data->materials[i];
        Material material;

        if (gltfMaterial.has_pbr_metallic_roughness) {
            if (gltfMaterial.pbr_metallic_roughness.base_color_texture.texture)
                material.baseColorIdx = textureOffset + cgltf_texture_index(data, gltfMaterial.pbr_metallic_roughness.base_color_texture.texture);

            material.baseColorFactor = vec4(
                gltfMaterial.pbr_metallic_roughness.base_color_factor[0], gltfMaterial.pbr_metallic_roughness.base_color_factor[1], gltfMaterial.pbr_metallic_roughness.base_color_factor[2],
                gltfMaterial.pbr_metallic_roughness.base_color_factor[3]
            );

            if (gltfMaterial.pbr_metallic_roughness.metallic_roughness_texture.texture) {
                material.metallicRoughnessIdx = textureOffset + cgltf_texture_index(data, gltfMaterial.pbr_metallic_roughness.metallic_roughness_texture.texture);

                material.metallicFactor = gltfMaterial.pbr_metallic_roughness.metallic_factor;
            }
        }

        if (gltfMaterial.normal_texture.texture) {
            material.normalIdx = textureOffset + cgltf_texture_index(data, gltfMaterial.normal_texture.texture);
        }

        if (gltfMaterial.emissive_texture.texture) {
            material.emissiveIdx = textureOffset + cgltf_texture_index(data, gltfMaterial.emissive_texture.texture);

            // m.emissiveFactor = vec3(material.emissive_factor[0], material.emissive_factor[1], material.emissive_factor[2]);
        }

        resourceManager.addMaterial(material);
    }
}

void loadTextures(vulkan::Graphics &graphics, ResourceManager &resourceManager, std::filesystem::path dir, cgltf_data *data)
{
    // XXX: this only works with separate image file, and not something like glb embedded
    for (size_t i = 0; i < data->textures_count; i++) {
        cgltf_texture gltfTexture = data->textures[i];

        std::filesystem::path file = dir / gltfTexture.image->uri;

        vulkan::Image texture;
        graphics.createImageFromFile(&texture, file);
        resourceManager.addImage(texture);
    }
}

void loadAnimations(Scene &scene, cgltf_data *data)
{
    // scene.animations.resize(data->animations_count);
    // for (size_t i = 0; i < data->animations_count; i++) {
    //     Animation &animation = scene.animations[i];
    //     cgltf_animation gltfAnimation = data->animations[i];

    //     animation.name = gltfAnimation.name;

    //     // channels
    //     animation.channels.resize(gltfAnimation.channels_count);
    //     for (size_t j = 0; j < gltfAnimation.channels_count; j++) {
    //         AnimationChannel channel;
    //         cgltf_animation_channel gltfChannel = gltfAnimation.channels[j];

    //         switch (gltfChannel.target_path) {
    //             case cgltf_animation_path_type_translation:
    //                 channel.targetType = AnimationTargetType::translation;
    //             case cgltf_animation_path_type_rotation:
    //                 channel.targetType = AnimationTargetType::rotation;
    //             case cgltf_animation_path_type_scale:
    //                 channel.targetType = AnimationTargetType::scale;
    //             case cgltf_animation_path_type_weights:
    //                 channel.targetType = AnimationTargetType::weights;
    //             default:
    //                 channel.targetType = AnimationTargetType::invalid;
    //         }

    //         channel.target = getNodeByIndex(scene, cgltf_node_index(data, gltfChannel.target_node));

    //         animation.channels[j] = channel;
    //     }

    //     // samplers
    //     animation.samplers.resize(gltfAnimation.samplers_count);
    //     for (size_t j = 0; j < gltfAnimation.samplers_count; j++) {
    //         AnimationSampler sampler;
    //         cgltf_animation_sampler gltfSampler = gltfAnimation.samplers[j];

    //         // inputs
    //         if (gltfSampler.input) {
    //             sampler.inputs.resize(gltfSampler.input->count);
    //             std::vector<float> inputs(sampler.inputs.size());
    //             cgltf_accessor_unpack_floats(gltfSampler.input, inputs.data(), inputs.size());

    //             for (size_t k = 0; k < sampler.inputs.size(); k++) {
    //                 sampler.inputs[k] = inputs[k];

    //                 // figure out start and end of animation
    //                 if (inputs[k] < animation.start)
    //                     animation.start = inputs[k];

    //                 if (inputs[k] > animation.end)
    //                     animation.end = inputs[k];
    //             }
    //         }

    //         // outputs
    //         if (gltfSampler.output) {
    //             sampler.outputs.resize(gltfSampler.output->count);

    //             std::vector<float> temp(gltfSampler.output->count * 4);

    //             for (size_t k = 0; k < sampler.outputs.size(); k++) {
    //                 if (gltfSampler.output->type == cgltf_type_vec4) {
    //                     cgltf_accessor_unpack_floats(gltfSampler.output, temp.data(), gltfSampler.output->count * 4);
    //                     sampler.outputs[k].x = temp[k * 4 + 0];
    //                     sampler.outputs[k].y = temp[k * 4 + 1];
    //                     sampler.outputs[k].z = temp[k * 4 + 2];
    //                     sampler.outputs[k].w = temp[k * 4 + 3];
    //                 } else if (gltfSampler.output->type == cgltf_type_vec3) {
    //                     cgltf_accessor_unpack_floats(gltfSampler.output, temp.data(), gltfSampler.output->count * 3);
    //                     sampler.outputs[k].x = temp[k * 3 + 0];
    //                     sampler.outputs[k].y = temp[k * 3 + 1];
    //                     sampler.outputs[k].z = temp[k * 3 + 2];
    //                     sampler.outputs[k].w = 0.0f;
    //                 }
    //             }
    //         }

    //         animation.samplers[j] = sampler;
    //     }
    // }
}

void loadSkins(Scene &scene, cgltf_data *data)
{
    scene.skins.resize(data->skins_count);
    for (size_t i = 0; i < data->skins_count; i++) {
        Skin &skin = scene.skins[i];
        cgltf_skin gltfSkin = data->skins[i];

        skin.name = gltfSkin.name;

        // joints
        skin.joints.resize(gltfSkin.joints_count);
        for (size_t j = 0; j < gltfSkin.joints_count; j++) {
            skin.joints[j] = cgltf_node_index(data, gltfSkin.joints[j]);
        }

        if (gltfSkin.skeleton)
            skin.skeleton = cgltf_node_index(data, gltfSkin.skeleton);

        // inverse bind matrices
        if (gltfSkin.inverse_bind_matrices) {
            assert(gltfSkin.inverse_bind_matrices->type == cgltf_type_mat4);
            std::vector<float> temp(gltfSkin.inverse_bind_matrices->count * 16);
            cgltf_accessor_unpack_floats(gltfSkin.inverse_bind_matrices, temp.data(), temp.size());

            skin.inverseBindMatrices.resize(gltfSkin.inverse_bind_matrices->count);
            for (size_t j = 0; j < skin.inverseBindMatrices.size(); j++) {
                skin.inverseBindMatrices[j] = {
                    temp[j * 16 + 0], temp[j * 16 + 1], temp[j * 16 + 2],  temp[j * 16 + 3],  temp[j * 16 + 4],  temp[j * 16 + 5],  temp[j * 16 + 6],  temp[j * 16 + 7],
                    temp[j * 16 + 8], temp[j * 16 + 9], temp[j * 16 + 10], temp[j * 16 + 11], temp[j * 16 + 12], temp[j * 16 + 13], temp[j * 16 + 14], temp[j * 16 + 15],
                };
            }
        }
    }
}

Transform loadTransform(cgltf_node *node)
{
    if (!node)
        return Transform();

    glm::vec3 position = vec3(0.0);
    glm::quat rotation = quat(1.0, 0.0, 0.0, 0.0);
    glm::vec3 scale = vec3(1.0);

    if (node->has_matrix) {
        mat4 matrix;
        cgltf_node_transform_world(node, &matrix[0][0]);

        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(matrix, scale, rotation, position, skew, perspective);
    } else {
        if (node->has_translation)
            position = glm::make_vec3(node->translation);

        if (node->has_scale)
            scale = glm::make_vec3(node->scale);

        if (node->has_rotation)
            rotation = glm::make_quat(node->rotation);
    }

    return Transform(position, rotation, scale);
}

} // namespace rebirth::gltf