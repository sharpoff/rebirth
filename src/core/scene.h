#pragma once

/*

#include "core/animation.h"
#include "core/material.h"
#include "core/mesh.h"
#include "render/render_types.h"

struct Skin
{
    String name;
    int skeletonIndex = -1;
    Vector<int> joints;
    Vector<mat4> inverseBindMatrices;

    uint32_t jointMatrixIndex;
};

struct SceneNode
{
    int parentIndex = -1;
    Vector<SceneNode> children;
    Mesh *mesh = nullptr;

    String name = "Node";
    mat4 transform = mat4::identity();
    Skin *skin = nullptr;
    int index = -1;
};

class Scene
{
public:
    mat4 getNodeWorldMatrix(SceneNode *node);
    Animation *getAnimationByName(String name);

    String name = "Model";
    mat4 transform = mat4::identity();

    Vector<SceneNode> nodes;
    Vector<Mesh> meshes;
    Vector<Skin> skins;
    Vector<GPUMaterial> materials;
    Vector<Image*> textures;
    Vector<Animation> animations;

private:
    void updateJoints(SceneNode *node);

    SceneNode *getNodeByIndex(int index);
    SceneNode *searchNode(SceneNode *node, int index);
};

*/