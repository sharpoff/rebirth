#pragma once

#include <rebirth/types/camera.h>
#include <rebirth/math/transform.h>
#include <rebirth/types/animation.h>
#include <rebirth/types/material.h>
#include <rebirth/types/mesh.h>
#include <rebirth/types/model.h>

struct Skin
{
    std::string name;
    SceneNodeID skeletonId = SceneNodeID::Invalid;
    std::vector<SceneNodeID> jointIds;
    std::vector<mat4> inverseBindMatrices;

    uint32_t jointMatrixIndex;
};

struct SceneNode
{
    std::string name = "Node";
    Transform localTransform = Transform();
    SceneNodeID id = SceneNodeID::Invalid;
    SkinID skinId = SkinID::Invalid;

    SceneNodeID parentId = SceneNodeID::Invalid;
    std::vector<SceneNode> children;
    Model model;
};

class Scene
{
public:
    std::string name;
    Transform transform = Transform();
    std::vector<SceneNode> nodes;

    std::vector<Skin> skins;
    std::vector<Animation> animations;
    std::vector<Camera> cameras;
    std::vector<LightID> lights;

    // XXX: is it working properly?
    void merge(Scene &scene);

    void updateAnimation(float deltaTime, std::string name);

    SceneNode *getNodeByIndex(SceneNodeID index);
    mat4 getNodeWorldMatrix(SceneNode *node);
    Animation *getAnimationByName(std::string name);

private:
    SceneNode *searchNode(SceneNode *node, SceneNodeID index);
    void updateJoints(SceneNode &node);
};