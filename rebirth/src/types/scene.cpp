#include <rebirth/types/scene.h>
#include <rebirth/vulkan/graphics.h>

namespace rebirth
{

void Scene::destroy(vulkan::Graphics &graphics)
{
    vkDeviceWaitIdle(graphics.getDevice());

    for (auto &skin : skins) {
        graphics.destroyBuffer(&skin.jointMatricesBuffer);
    }
}

void Scene::merge(Scene &scene)
{
    if (this == &scene) return;

    size_t nodesCount = nodes.size();
    size_t skinsCount = skins.size();

    std::function<void(SceneNode &)> mergeNode = [&](SceneNode &node) {
        node.localTransform = scene.transform * node.localTransform;
        node.index += node.index > -1 ? nodesCount : 0;
        node.skin += node.skin > -1 ? skinsCount : 0;
        node.parent += node.parent > -1 ? nodesCount : 0;

        for (auto &child : node.children) {
            mergeNode(child);
        }
    };

    for (auto &node : scene.nodes) {
        mergeNode(node);
        nodes.push_back(node);
    }

    for (auto &skin : scene.skins) {
        skin.skeleton += skin.skeleton > -1 ? nodesCount : 0;
        for (auto &joint : skin.joints) {
            joint += joint > -1 ? nodesCount : 0;
        }

        skins.push_back(skin);
    }

    for (auto &animation : scene.animations) {
        for (auto &channel : animation.channels) {
            channel.node += channel.node > -1 ? nodesCount : 0;
        }

        animations.push_back(animation);
    }
}

void Scene::updateAnimation(vulkan::Graphics &graphics, float deltaTime, std::string name)
{
    if (animations.size() < 1) return;

    Animation *animation = getAnimationByName(name);
    if (!animation) return;

    animation->currentTime += deltaTime;
    if (animation->currentTime > animation->end) {
        animation->currentTime -= animation->end;
    }

    for (auto &channel : animation->channels) {
        if (channel.sampler == -1)
            continue;

        AnimationSampler &sampler = animation->samplers[channel.sampler];

        for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
            if ((animation->currentTime >= sampler.inputs[i]) &&
                (animation->currentTime <= sampler.inputs[i + 1])) {
                float step = (animation->currentTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
                SceneNode *node = getNodeByIndex(channel.node);

                if (channel.path == AnimationPath::translation) {
                    node->localTransform.setPosition(
                        glm::mix(sampler.outputs[i], sampler.outputs[i + 1], step)
                    );
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
                        glm::mix(sampler.outputs[i], sampler.outputs[i + 1], step)
                    );
                }
            }
        }
    }

    for (auto &node : nodes) {
        updateJoints(graphics, node);
    }
}

SceneNode *Scene::getNodeByIndex(int index)
{
    SceneNode *found = nullptr;
    for (auto &node : nodes) {
        found = searchNode(&node, index);
        if (found) break;
    }

    return found;
}

SceneNode *Scene::searchNode(SceneNode *node, int index)
{
    SceneNode *found = nullptr;
    if (node->index == index)
        return node;
    
    for (auto &child : node->children) {
        found = searchNode(&child, index);
        if (found) break;
    }

    return found;
}

mat4 Scene::getNodeWorldMatrix(SceneNode *node)
{
    if (!node || node->parent < 0) return mat4(1.0f);

    mat4 worldMatrix = node->localTransform.getModelMatrix();
    SceneNode *parent = getNodeByIndex(node->parent);
    while (parent) {
        worldMatrix = parent->localTransform.getModelMatrix() * worldMatrix;
        parent = getNodeByIndex(parent->parent);
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

void Scene::updateJoints(vulkan::Graphics &graphics, SceneNode &node)
{
    if (node.skin > -1) {
        mat4 inverseTransform = glm::inverse(getNodeWorldMatrix(&node));
        Skin &skin = skins[node.skin];

        size_t jointsCount = skin.joints.size();
        std::vector<mat4> jointMatrices(jointsCount);

        for (size_t i = 0; i < jointsCount; i++) {
            SceneNode *joint = getNodeByIndex(skin.joints[i]);

            jointMatrices[i] = getNodeWorldMatrix(joint) * skin.inverseBindMatrices[i];
            jointMatrices[i] = inverseTransform * jointMatrices[i];
        }

        // Update ssbo
        graphics.uploadBuffer(
            skin.jointMatricesBuffer, jointMatrices.data(), jointMatrices.size() * sizeof(mat4)
        );
    }

    for (auto &child : node.children) {
        updateJoints(graphics, child);
    }
}

} // namespace rebirth