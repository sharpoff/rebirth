#pragma once

#include <filesystem>

#include <rebirth/core/animation.h>
#include <rebirth/core/light.h>
#include <rebirth/core/scene.h>

#include <cgltf.h>

class Renderer;

namespace gltf
{
    bool loadScene(Renderer &renderer, Scene &scene, std::filesystem::path file);

    bool loadGltfNode(Renderer &renderer, Scene &scene, SceneNode &node, cgltf_data *data, cgltf_node *gltfNode);
    bool loadGltfMesh(Renderer &renderer, Scene &scene, Mesh &mesh, cgltf_data *data, cgltf_mesh *gltfMesh);

    size_t loadVertices(eastl::vector<Vertex> &vertices, cgltf_primitive prim);
    size_t loadIndices(eastl::vector<uint32_t> &indices, cgltf_primitive prim);

    void loadGltfMaterials(Renderer &renderer, cgltf_data *data);
    void loadGltfTextures(Renderer &renderer, std::filesystem::path dir, cgltf_data *data);

    void loadGltfAnimations(Scene &scene, cgltf_data *data);
    void loadGltfSkins(Scene &scene, cgltf_data *data);

    bool loadGltfLight(Light &light, mat4 worldMatrix, cgltf_light *gltfLight);

    bool loadGltfTransform(mat4 transform, cgltf_node *node, bool world);
} // namespace gltf