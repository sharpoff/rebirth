#pragma once

#include <filesystem>

#include "core/light.h"
#include "core/model.h"
#include "core/vertex.h"

#include <cgltf.h>

namespace vulkan
{
    class Graphics;
}

namespace gltf
{
    //
    // Loading
    //
    bool loadModel(Model &result, vulkan::Graphics &graphics, std::filesystem::path modelFile);

    void loadGltfNode(Model &model, ModelNode &modelNode, cgltf_data *data, cgltf_node *gltfNode);
    void loadGltfMesh(Mesh &mesh, mat4 transform, cgltf_data *data, cgltf_mesh *gltfMesh);

    eastl::vector<Vertex> loadVertices(cgltf_primitive prim);
    eastl::vector<uint32_t> loadIndices(cgltf_primitive prim);

    void loadGltfMaterials(Model &model, cgltf_data *data, size_t textureOffset);
    void loadGltfTextures(Model &model, vulkan::Graphics &graphics, std::filesystem::path dir, cgltf_data *data);

    void loadGltfAnimations(Model &model, cgltf_data *data);
    void loadGltfSkins(Model &model, cgltf_data *data);

    void loadGltfLight(GPULight &light, mat4 worldMatrix, cgltf_light *gltfLight);

    mat4 loadGltfTransform(cgltf_node *node, bool world);
} // namespace gltf