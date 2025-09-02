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
    int skeleton = -1; // idx to a node
    std::vector<int> joints; // indices to a node
    std::vector<mat4> inverseBindMatrices;

    vulkan::Buffer jointMatricesBuffer;
};

struct SceneNode
{
    std::string name = "Node";
    Transform localTransform = Transform();
    int index = -1; // idx to a node
    int skin = -1; // idx to a skin

    int parent = -1; // idx to a node
    std::vector<SceneNode> children;
    SceneMesh mesh;
};

class Scene
{
public:
    std::string name;
    Transform transform = Transform();
    std::vector<SceneNode> nodes;
    std::vector<Vertex> vertices;

    std::vector<Skin> skins;
    std::vector<Animation> animations;
    std::string currentAnimation = "";

    void destroy(vulkan::Graphics &graphics);

    void merge(Scene &scene);

    void updateAnimation(vulkan::Graphics &graphics, float deltaTime);

    SceneNode *getNodeByIndex(int index);
    mat4 getNodeWorldMatrix(SceneNode *node);
    Animation *getAnimationByName(std::string name);

private:
    SceneNode *searchNode(SceneNode *node, int index);
    void updateJoints(vulkan::Graphics &graphics, SceneNode &node);
};

} // namespace rebirth