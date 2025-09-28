#pragma once

#include <filesystem>

#include <rebirth/types/light.h>
#include <rebirth/types/animation.h>
#include <rebirth/types/scene.h>

#include <cgltf.h>

namespace vulkan
{
    class Graphics;
}

namespace gltf
{
    bool loadScene(Scene &scene, std::filesystem::path file);
    bool loadModel(Model &model, std::filesystem::path file);

    bool loadGltfNode(Scene &scene, SceneNode &node, cgltf_data *data, cgltf_node *gltfNode);
    bool loadGltfMesh(Model &sceneMesh, cgltf_data *data, cgltf_mesh *gltfMesh);

    void loadGltfNodes(Scene &scene, cgltf_scene *gltfScene, cgltf_data *data);

    void loadVertices(std::vector<Vertex> &vertices, cgltf_primitive prim);
    void loadIndices(std::vector<uint32_t> &indices, size_t verticesCount, cgltf_primitive prim);

    void loadGltfMaterials(cgltf_data *data);
    void loadGltfTextures(std::filesystem::path dir, cgltf_data *data);

    void loadGltfAnimations(Scene &scene, cgltf_data *data);
    void loadGltfSkins(Scene &scene, cgltf_data *data);

    bool loadGltfCamera(Camera &camera, mat4 worldMatrix, cgltf_camera *gltfCamera);
    bool loadGltfLight(Light &light, mat4 worldMatrix, cgltf_light *gltfLight);

    bool loadGltfTransform(Transform &transform, cgltf_node *node, bool world);
} // namespace gltf