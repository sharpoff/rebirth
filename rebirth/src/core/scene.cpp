#include <rebirth/core/scene.h>

void Scene::updateAnimation(float deltaTime, eastl::string name)
{
    if (animations.size() < 1)
        return;

    Animation *animation = getAnimationByName(name);
    if (!animation)
        return;

    animation->currentTime += deltaTime;
    if (animation->currentTime > animation->end) {
        animation->currentTime -= animation->end;
    }

    for (auto &channel : animation->channels) {
        AnimationSampler &sampler = animation->samplers[channel.samplerIndex];

        for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
            if ((animation->currentTime >= sampler.inputs[i]) &&
                (animation->currentTime <= sampler.inputs[i + 1])) {
                float step = (animation->currentTime - sampler.inputs[i]) /
                             (sampler.inputs[i + 1] - sampler.inputs[i]);
                SceneNode *node = getNodeByIndex(channel.nodeIndex);

                if (!node)
                    continue;

                if (channel.path == AnimationPath::translation) {
                    node->transform *= glm::translate(vec3(glm::mix(sampler.outputs[i], sampler.outputs[i + 1], step)));
                }

                if (channel.path == AnimationPath::rotation) {
                    glm::quat q1;
                    q1.x = sampler.outputs[i].x;
                    q1.y = sampler.outputs[i].y;
                    q1.z = sampler.outputs[i].z;
                    q1.w = sampler.outputs[i].w;

                    glm::quat q2;
                    q2.x = sampler.outputs[i + 1].x;
                    q2.y = sampler.outputs[i + 1].y;
                    q2.z = sampler.outputs[i + 1].z;
                    q2.w = sampler.outputs[i + 1].w;

                    node->transform *= mat4(glm::normalize(glm::slerp(q1, q2, step)));
                }

                if (channel.path == AnimationPath::scale) {
                    node->transform = glm::scale(vec3(glm::mix(sampler.outputs[i], sampler.outputs[i + 1], step)));
                }
            }
        }
    }

    for (auto &node : nodes) {
        updateJoints(node);
    }
}

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

void Scene::updateJoints(SceneNode &node)
{
    if (node.skinIndex > -1) {
        mat4 inverseTransform = glm::inverse(getNodeWorldMatrix(&node));
        Skin &skin = skins[node.skinIndex];

        size_t jointsCount = skin.joints.size();
        eastl::vector<mat4> jointMatrices(jointsCount);

        for (size_t i = 0; i < jointsCount; i++) {
            SceneNode *joint = getNodeByIndex(skin.joints[i]);
            if (!joint)
                continue;

            jointMatrices[i] = getNodeWorldMatrix(joint) * skin.inverseBindMatrices[i];
            jointMatrices[i] = inverseTransform * jointMatrices[i];
        }
    }

    for (auto &child : node.children) {
        updateJoints(child);
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