#pragma once

#include <filesystem>

#include <rebirth/animation.h>
#include <rebirth/resource_manager.h>
#include <rebirth/scene.h>
#include <rebirth/types.h>

#include <cgltf.h>

namespace rebirth::vulkan
{
class Graphics;
}

namespace rebirth::gltf
{

bool loadScene(Scene *scene, vulkan::Graphics &graphics, ResourceManager &resourceManager, std::filesystem::path file);
void loadNode(vulkan::Graphics &graphics, ResourceManager &resourceManager, Scene &scene, SceneNode &node, cgltf_data *data, cgltf_node *gltfNode, uint32_t materialOffset);
void loadMesh(vulkan::Graphics &graphics, ResourceManager &resourceManager, SceneNode &node, cgltf_data *data, cgltf_mesh *gltfMesh, uint32_t materialOffset);
void loadMaterials(ResourceManager &resourceManager, cgltf_data *data);
void loadTextures(vulkan::Graphics &graphics, ResourceManager &resourceManager, std::filesystem::path dir, cgltf_data *data);
void loadAnimations(Scene &scene, cgltf_data *data);
void loadSkins(Scene &scene, cgltf_data *data);

Transform loadTransform(cgltf_node *node);

} // namespace rebirth::gltf