#include <rebirth/gltf.h>
#include <rebirth/vulkan/graphics.h>

namespace rebirth::gltf
{

std::optional<Scene> loadScene(vulkan::Graphics &graphics, std::vector<std::filesystem::path> &texturePaths, std::vector<Material> &materials, std::filesystem::path file)
{
    cgltf_options options = {};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, file.c_str(), &data);
    if (result != cgltf_result_success) {
        printf("Failed to load gltf scene.\n");
        return std::nullopt;
    }

    result = cgltf_load_buffers(&options, data, file.c_str());

    size_t textureOffset = texturePaths.size();
    size_t materialOffset = materials.size();

    // load materials
    for (size_t i = 0; i < data->materials_count; i++) {
        cgltf_material gltfMaterial = data->materials[i];
        Material mat;

        if (gltfMaterial.has_pbr_metallic_roughness) {
            if (gltfMaterial.pbr_metallic_roughness.base_color_texture.texture)
                mat.baseColorIdx = textureOffset + cgltf_texture_index(data, gltfMaterial.pbr_metallic_roughness.base_color_texture.texture);

            mat.baseColorFactor = vec4(
                gltfMaterial.pbr_metallic_roughness.base_color_factor[0], gltfMaterial.pbr_metallic_roughness.base_color_factor[1], gltfMaterial.pbr_metallic_roughness.base_color_factor[2],
                gltfMaterial.pbr_metallic_roughness.base_color_factor[3]
            );

            if (gltfMaterial.pbr_metallic_roughness.metallic_roughness_texture.texture) {
                mat.metallicRoughnessIdx = textureOffset + cgltf_texture_index(data, gltfMaterial.pbr_metallic_roughness.metallic_roughness_texture.texture);

                mat.metallicFactor = gltfMaterial.pbr_metallic_roughness.metallic_factor;
            }
        }

        if (gltfMaterial.normal_texture.texture) {
            mat.normalIdx = textureOffset + cgltf_texture_index(data, gltfMaterial.normal_texture.texture);
        }

        if (gltfMaterial.emissive_texture.texture) {
            mat.emissiveIdx = textureOffset + cgltf_texture_index(data, gltfMaterial.emissive_texture.texture);

            // m.emissiveFactor = vec3(material.emissive_factor[0], material.emissive_factor[1], material.emissive_factor[2]);
        }

        materials.push_back(mat);
    }

    // load textures
    for (size_t i = 0; i < data->textures_count; i++) {
        cgltf_texture texture = data->textures[i];

        std::filesystem::path path = file.parent_path() / texture.image->uri;
        texturePaths.push_back(path);
    }

    Scene scene;
    cgltf_scene *root = data->scene;

    // recursively load nodes
    for (size_t i = 0; i < root->nodes_count; i++)
        loadNode(graphics, scene, data, root->nodes[i], materialOffset);

    cgltf_free(data);
    return scene;
}

void loadNode(vulkan::Graphics &graphics, Scene &scene, cgltf_data *data, cgltf_node *gltfNode, uint32_t materialOffset)
{
    if (!gltfNode)
        return;

    if (gltfNode->mesh) {
        mat4 nodeMatrix;
        cgltf_node_transform_world(gltfNode, &nodeMatrix[0][0]);
        loadMesh(graphics, scene, nodeMatrix, data, gltfNode->mesh, materialOffset);
    }

    // recursively load child nodes
    for (size_t i = 0; i < gltfNode->children_count; i++) {
        if (gltfNode->children[i])
            loadNode(graphics, scene, data, gltfNode->children[i], materialOffset);
    }
}

void loadMesh(vulkan::Graphics &graphics, Scene &scene, mat4 nodeMatrix, cgltf_data *data, cgltf_mesh *gltfMesh, uint32_t materialOffset)
{
    for (size_t i = 0; i < gltfMesh->primitives_count; i++) {
        cgltf_primitive prim = gltfMesh->primitives[i];

        // load vertices
        std::vector<Vertex> vertices(prim.attributes[0].data->count);
        size_t vertexCount = vertices.size();
        std::vector<float> temp(vertexCount * 4);

        if (const cgltf_accessor *pos = cgltf_find_accessor(&prim, cgltf_attribute_type_position, 0)) {
            assert(cgltf_num_components(pos->type) == 3);
            cgltf_accessor_unpack_floats(pos, temp.data(), vertexCount * 3);

            for (size_t i = 0; i < vertexCount; i++) {
                vertices[i].position.x = temp[i * 3 + 0];
                vertices[i].position.y = temp[i * 3 + 1];
                vertices[i].position.z = temp[i * 3 + 2];
            }
        }

        if (const cgltf_accessor *uv = cgltf_find_accessor(&prim, cgltf_attribute_type_texcoord, 0)) {
            assert(cgltf_num_components(uv->type) == 2);
            cgltf_accessor_unpack_floats(uv, temp.data(), vertexCount * 2);

            for (size_t i = 0; i < vertexCount; i++) {
                vertices[i].uv_x = temp[i * 2 + 0];
                vertices[i].uv_y = temp[i * 2 + 1];
            }
        }

        if (const cgltf_accessor *normal = cgltf_find_accessor(&prim, cgltf_attribute_type_normal, 0)) {
            assert(cgltf_num_components(normal->type) == 3);
            cgltf_accessor_unpack_floats(normal, temp.data(), vertexCount * 3);

            for (size_t i = 0; i < vertexCount; i++) {
                vertices[i].normal.x = temp[i * 3 + 0];
                vertices[i].normal.y = temp[i * 3 + 1];
                vertices[i].normal.z = temp[i * 3 + 2];
            }
        }

        // load meshIndices
        std::vector<uint32_t> indices(prim.indices->count);
        cgltf_accessor_unpack_indices(prim.indices, indices.data(), 4, indices.size());

        Mesh mesh;
        mesh.indexCount = indices.size();
        mesh.matrix = nodeMatrix;
        mesh.materialIdx = materialOffset + cgltf_material_index(data, prim.material);
        graphics.createBuffer(&mesh.vertexBuffer, vertices.size() * sizeof(Vertex), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        graphics.createBuffer(&mesh.indexBuffer, indices.size() * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

        graphics.uploadBuffer(mesh.vertexBuffer, vertices.data(), vertices.size() * sizeof(Vertex));
        graphics.uploadBuffer(mesh.indexBuffer, indices.data(), indices.size() * sizeof(uint32_t));

        scene.meshes.push_back(mesh);
    }
}

} // namespace rebirth::gltf