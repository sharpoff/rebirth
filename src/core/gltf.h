#pragma once

#include <filesystem>

#include "core/light.h"
#include "core/scene.h"
#include "core/vertex.h"

#include "render/render_device.h"

#include <cgltf.h>

namespace gltf
{
    bool loadModel(Scene &result, RenderDevice *renderDevice, std::filesystem::path modelFile);

    void loadGltfNode(Scene &model, SceneNode &modelNode, cgltf_data *data, cgltf_node *gltfNode);
    void loadGltfMesh(Mesh &mesh, mat4 transform, cgltf_data *data, cgltf_mesh *gltfMesh);

    eastl::vector<Vertex>   loadVertices(cgltf_primitive prim);
    eastl::vector<uint32_t> loadIndices(cgltf_primitive prim);

    void loadGltfMaterials(Scene &model, cgltf_data *data, size_t textureOffset);
    void loadGltfTextures(Scene &model, RenderDevice *renderDevice, std::filesystem::path dir, cgltf_data *data);

    void loadGltfAnimations(Scene &model, cgltf_data *data);
    void loadGltfSkins(Scene &model, cgltf_data *data);

    void loadGltfLight(GPULight &light, mat4 worldMatrix, cgltf_light *gltfLight);

    mat4 loadGltfTransform(cgltf_node *node, bool world);
} // namespace gltf