#pragma once

#include <filesystem>
#include <optional>

#include <rebirth/types.h>

#include <cgltf.h>

namespace rebirth::vulkan
{
class Graphics;
}

namespace rebirth::gltf
{

std::optional<Scene> loadScene(vulkan::Graphics &graphics, std::vector<std::filesystem::path> &texturePaths, std::vector<Material> &materials, std::filesystem::path file);
void loadNode(vulkan::Graphics &graphics, Scene &scene, cgltf_data *data, cgltf_node *gltfNode, uint32_t materialOffset);
void loadMesh(vulkan::Graphics &graphics, Scene &scene, mat4 nodeMatrix, cgltf_data *data, cgltf_mesh *gltfMesh, uint32_t materialOffset);

} // namespace rebirth::gltf