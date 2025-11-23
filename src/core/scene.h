#pragma once

#include "core/animation.h"
#include "core/material.h"
#include "core/mesh.h"
#include "render/graphics_types.h"

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
    Mesh *mesh = nullptr;

    eastl::string name = "Node";
    mat4 transform = mat4(1.0f);
    Skin *skin = nullptr;
    int index = -1;
};

class Scene
{
public:
    mat4 getNodeWorldMatrix(SceneNode *node);
    Animation *getAnimationByName(eastl::string name);

    eastl::string name = "Model";
    mat4 transform = mat4(1.0f);

    eastl::vector<SceneNode> nodes;
    eastl::vector<Mesh> meshes;
    eastl::vector<Skin> skins;
    eastl::vector<GPUMaterial> materials;
    eastl::vector<Texture*> textures;
    eastl::vector<Animation> animations;

private:
    void updateJoints(SceneNode *node);

    SceneNode *getNodeByIndex(int index);
    SceneNode *searchNode(SceneNode *node, int index);
};