#include "core/scene.h"

mat4 Scene::getNodeWorldMatrix(SceneNode *node)
{
    if (!node)
        return mat4(1.0f);

    mat4 worldMatrix = node->transform;
    SceneNode *parent = getNodeByIndex(node->parentIndex);
    while (parent) {
        worldMatrix = parent->transform * worldMatrix;
        parent = getNodeByIndex(parent->parentIndex);
    }

    return worldMatrix;
}

Animation *Scene::getAnimationByName(eastl::string name)
{
    for (auto &animation : animations) {
        if (animation.name == name)
            return &animation;
    }

    return nullptr;
}

void Scene::updateJoints(SceneNode *node)
{
    if (!node)
        return;

    if (node->skin) {
        mat4 inverseTransform = glm::inverse(getNodeWorldMatrix(node));

        size_t jointsCount = node->skin->joints.size();
        eastl::vector<mat4> jointMatrices(jointsCount);

        for (size_t i = 0; i < jointsCount; i++) {
            SceneNode *joint = getNodeByIndex(node->skin->joints[i]);
            if (!joint)
                continue;

            jointMatrices[i] = getNodeWorldMatrix(joint) * node->skin->inverseBindMatrices[i];
            jointMatrices[i] = inverseTransform * jointMatrices[i];
        }
    }

    for (auto &child : node->children) {
        updateJoints(&child);
    }
}

SceneNode *Scene::getNodeByIndex(int index)
{
    if (index <= -1)
        return nullptr;

    SceneNode *found = nullptr;
    for (auto &node : nodes) {
        found = searchNode(&node, index);
        if (found)
            break;
    }

    return found;
}

SceneNode *Scene::searchNode(SceneNode *node, int index)
{
    if (index <= -1)
        return nullptr;

    SceneNode *found = nullptr;
    if (node->index == index)
        return node;

    for (auto &child : node->children) {
        found = searchNode(&child, index);
        if (found)
            break;
    }

    return found;
}