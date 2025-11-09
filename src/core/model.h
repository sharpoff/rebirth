#pragma once

#include "core/animation.h"
#include "core/material.h"
#include "core/mesh.h"
#include "graphics/vulkan/resources.h"

struct ModelNode;

struct Skin
{
    eastl::string name;
    int skeletonIndex = -1;
    eastl::vector<int> joints;
    eastl::vector<mat4> inverseBindMatrices;

    uint32_t jointMatrixIndex;
};

struct ModelNode
{
    int parentIndex = -1;
    eastl::vector<ModelNode> children;
    Mesh *mesh = nullptr;

    eastl::string name = "Node";
    mat4 transform = mat4(1.0f);
    Skin *skin = nullptr;
    int index = -1;
};

namespace vulkan
{
    class Graphics;
}

class Model
{
public:
    void updateAnimation(float deltaTime, eastl::string name);

    mat4 getNodeWorldMatrix(ModelNode *node);
    Animation *getAnimationByName(eastl::string name);

    eastl::string name = "Model";
    mat4 transform = mat4(1.0f);

    eastl::vector<ModelNode> nodes;
    eastl::vector<Mesh> meshes;
    eastl::vector<Skin> skins;
    eastl::vector<GPUMaterial> materials;
    eastl::vector<vulkan::Image> textures;
    eastl::vector<Animation> animations;

private:
    void updateJoints(ModelNode *node);

    ModelNode *getNodeByIndex(int index);
    ModelNode *searchNode(ModelNode *node, int index);
};