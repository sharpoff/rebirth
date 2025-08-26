#pragma once

#include <rebirth/transform.h>
#include <rebirth/vulkan/resources.h>

#include <string>

using MeshIdx = size_t;
using MaterialIdx = size_t;

namespace rebirth
{

struct Animation;

struct SceneMesh
{
    std::vector<MeshIdx> primitives;
};

struct Skin
{
    std::string name;
    size_t skeleton = -1;       // idx to a node
    std::vector<size_t> joints; // idx to a node
    std::vector<mat4> inverseBindMatrices;
};

struct SceneNode
{
    Transform transform = Transform();
    size_t index = 0;
    int skin = -1; // idx to a skin

    SceneNode *parent = nullptr;
    std::vector<SceneNode> children;
    SceneMesh mesh;
};

struct Scene
{
    std::string name;
    Transform transform = Transform();
    std::vector<SceneNode> nodes;

    std::vector<Skin> skins;
    std::vector<Animation> animations;
};

} // namespace rebirth