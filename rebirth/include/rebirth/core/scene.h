#pragma once

#include <rebirth/core/animation.h>
#include <rebirth/core/material.h>
#include <rebirth/core/mesh.h>
#include <rebirth/core/camera.h>

#include <rebirth/graphics/vulkan/resources.h>

struct SceneNode;

struct Skin
{
    eastl::string name;
    int skeletonIndex = -1;
    eastl::vector<int> joints;
    eastl::vector<mat4> inverseBindMatrices;

    uint32_t jointMatrixIndex;
};

struct SceneNode
{
    int parentIndex = -1;
    eastl::vector<SceneNode> children;
    Mesh mesh;

    eastl::string name = "Node";
    mat4 transform = mat4(1.0f);
    int skinIndex = -1;
    int index = -1;
};

class Scene
{
public:
    eastl::string name;
    mat4 transform = mat4(1.0f);
    eastl::vector<SceneNode> nodes;

    eastl::vector<Skin> skins;
    eastl::vector<Animation> animations;

    // void merge(Scene &scene);

    void updateAnimation(float deltaTime, eastl::string name);

    mat4 getNodeWorldMatrix(SceneNode *node);
    Animation *getAnimationByName(eastl::string name);

private:
    void updateJoints(SceneNode &node);

    SceneNode *getNodeByIndex(int index);
    SceneNode *searchNode(SceneNode *node, int index);
};