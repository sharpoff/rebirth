#include <rebirth/types/scene.h>

// XXX: is it working properly?
void Scene::merge(Scene &scene)
{
    if (this == &scene)
        return;

    size_t nodesCount = nodes.size();
    size_t skinsCount = skins.size();

    std::function<void(SceneNode &)> mergeNode = [&](SceneNode &node) {
        node.localTransform = scene.transform * node.localTransform;
        if (node.id != SceneNodeID::Invalid) {
            node.id = SceneNodeID(ID(node.id) + nodesCount);
        }
        if (node.skinId != SkinID::Invalid) {
            node.skinId = SkinID(ID(node.skinId) + skinsCount);
        }
        if (node.parentId != SceneNodeID::Invalid) {
            node.parentId = SceneNodeID(ID(node.parentId) + nodesCount);
        }

        for (auto &child : node.children) {
            mergeNode(child);
        }
    };

    for (auto &node : scene.nodes) {
        mergeNode(node);
        nodes.push_back(node);
    }

    for (auto &skin : scene.skins) {
        if (skin.skeletonId != SceneNodeID::Invalid) {
            skin.skeletonId = SceneNodeID(ID(skin.skeletonId) + nodesCount);
        }
        for (auto &jointId : skin.jointIds) {
            if (jointId != SceneNodeID::Invalid) {
                jointId = SceneNodeID(ID(jointId) + nodesCount);
            }
        }

        skins.push_back(skin);
    }

    for (auto &animation : scene.animations) {
        for (auto &channel : animation.channels) {
            if (channel.nodeId != SceneNodeID::Invalid) {
                channel.nodeId = SceneNodeID(ID(channel.nodeId) + nodesCount);
            }
        }

        animations.push_back(animation);
    }
}

void Scene::updateAnimation(float deltaTime, std::string name)
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
        if (channel.samplerId == SamplerID::Invalid)
            continue;

        AnimationSampler &sampler = animation->samplers[ID(channel.samplerId)];

        for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
            if ((animation->currentTime >= sampler.inputs[i]) &&
                (animation->currentTime <= sampler.inputs[i + 1])) {
                float step = (animation->currentTime - sampler.inputs[i]) /
                             (sampler.inputs[i + 1] - sampler.inputs[i]);
                SceneNode *node = getNodeByIndex(channel.nodeId);

                if (channel.path == AnimationPath::translation) {
                    node->localTransform.setPosition(
                        glm::mix(sampler.outputs[i], sampler.outputs[i + 1], step));
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

                    node->localTransform.setRotation(glm::normalize(glm::slerp(q1, q2, step)));
                }

                if (channel.path == AnimationPath::scale) {
                    node->localTransform.setScale(
                        glm::mix(sampler.outputs[i], sampler.outputs[i + 1], step));
                }
            }
        }
    }

    for (auto &node : nodes) {
        updateJoints(node);
    }
}

SceneNode *Scene::getNodeByIndex(SceneNodeID index)
{
    if (index == SceneNodeID::Invalid)
        return nullptr;

    SceneNode *found = nullptr;
    for (auto &node : nodes) {
        found = searchNode(&node, index);
        if (found)
            break;
    }

    return found;
}

SceneNode *Scene::searchNode(SceneNode *node, SceneNodeID index)
{
    if (index == SceneNodeID::Invalid)
        return nullptr;

    SceneNode *found = nullptr;
    if (node->id == index)
        return node;

    for (auto &child : node->children) {
        found = searchNode(&child, index);
        if (found)
            break;
    }

    return found;
}

mat4 Scene::getNodeWorldMatrix(SceneNode *node)
{
    if (!node)
        return mat4(1.0f);

    mat4 worldMatrix = node->localTransform.getModelMatrix();
    SceneNode *parent = getNodeByIndex(node->parentId);
    while (parent) {
        worldMatrix = parent->localTransform.getModelMatrix() * worldMatrix;
        parent = getNodeByIndex(parent->parentId);
    }

    return worldMatrix;
}

Animation *Scene::getAnimationByName(std::string name)
{
    for (auto &animation : animations) {
        if (animation.name == name)
            return &animation;
    }

    return nullptr;
}

void Scene::updateJoints(SceneNode &node)
{
    if (node.skinId == SkinID::Invalid) {
        mat4 inverseTransform = glm::inverse(getNodeWorldMatrix(&node));
        Skin &skin = skins[ID(node.skinId)];

        size_t jointsCount = skin.jointIds.size();
        std::vector<mat4> jointMatrices(jointsCount);

        for (size_t i = 0; i < jointsCount; i++) {
            SceneNode *joint = getNodeByIndex(skin.jointIds[i]);

            jointMatrices[i] = getNodeWorldMatrix(joint) * skin.inverseBindMatrices[i];
            jointMatrices[i] = inverseTransform * jointMatrices[i];
        }

        // TODO: update global joints matrix
    }

    for (auto &child : node.children) {
        updateJoints(child);
    }
}