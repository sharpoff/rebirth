#pragma once

#include <filesystem>

#include "core/light.h"
#include "core/scene.h"
#include "core/vertex.h"

#include <cgltf.h>

namespace vulkan {
    class Graphics;
}

namespace gltf
{
    bool loadScene(vulkan::Graphics &graphics, Scene &scene, std::filesystem::path file);

    bool loadGltfNode(Scene &scene, SceneNode &node, cgltf_data *data, cgltf_node *gltfNode);
    void loadGltfMesh(Scene &scene, mat4 transform, cgltf_data *data, cgltf_mesh *gltfMesh);

    size_t loadVertices(eastl::vector<Vertex> &vertices, cgltf_primitive prim);
    size_t loadIndices(eastl::vector<uint32_t> &indices, cgltf_primitive prim);

    void loadGltfMaterials(cgltf_data *data);
    void loadGltfTextures(vulkan::Graphics &graphics, std::filesystem::path dir, cgltf_data *data);

    void loadGltfAnimations(Scene &scene, cgltf_data *data);
    void loadGltfSkins(Scene &scene, cgltf_data *data);

    void loadGltfLight(GPULight &light, mat4 worldMatrix, cgltf_light *gltfLight);

    mat4 loadGltfTransform(cgltf_node *node, bool world);
} // namespace gltf