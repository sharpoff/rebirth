#pragma once

#include <rebirth/math/transform.h>
#include <rebirth/types/material.h>
#include <rebirth/types/animation.h>
#include <rebirth/types/mesh.h>
#include <rebirth/vulkan/resources.h>

#include <string>

namespace rebirth::vulkan
{
class Graphics;
}

namespace rebirth
{

struct SceneNode;

struct SceneMesh
{
    std::vector<MeshID> primitives;
};

struct Skin
{
    std::string name;
    SceneNode *skeleton = nullptr;
    std::vector<SceneNode *> joints;
    std::vector<mat4> inverseBindMatrices;

    // only for skinned mesh
    vulkan::Buffer jointMatricesBuffer;
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

class Scene
{
public:
    std::string name;
    Transform transform = Transform();
    std::vector<SceneNode> nodes;

    std::vector<Skin> skins;
    std::vector<Animation> animations;

    void destroy();

    void merge(Scene &scene);

    void updateAnimation(float deltaTime);

    vulkan::Graphics *graphics;
private:
    void updateJoints(SceneNode &node);
};

} // namespace rebirth