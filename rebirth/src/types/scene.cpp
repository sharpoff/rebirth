#include <rebirth/types/scene.h>
#include <rebirth/vulkan/graphics.h>

namespace rebirth
{

void Scene::destroy()
{
    vkDeviceWaitIdle(graphics->getDevice());

    for (auto &skin :skins) {
        graphics->destroyBuffer(&skin.jointMatricesBuffer);
    }
}

void Scene::merge(Scene &scene)
{
    for (auto &node : scene.nodes) {
        node.transform = scene.transform * node.transform;
        if (node.skin > -1) {
            node.skin += skins.size();
        }

        nodes.push_back(node);
    }

    for (auto &skin : scene.skins) {
        skins.push_back(skin);
    }

    for (auto &animation : scene.animations) {
        animations.push_back(animation);
    }
}

void Scene::updateAnimation(float deltaTime)
{
    if (animations.size() < 1)
        return;

    auto &animation = animations[0];
    animation.currentTime += deltaTime;
    if (animation.currentTime > animation.end) {
        animation.currentTime -= animation.end;
    }

    for (auto &channel : animation.channels) {
        if (channel.sampler == -1)
            continue;

        AnimationSampler &sampler = animation.samplers[channel.sampler];

        for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
            if ((animation.currentTime >= sampler.inputs[i]) &&
                (animation.currentTime <= sampler.inputs[i + 1])) {
                float step = (animation.currentTime - sampler.inputs[i]) /
                             (sampler.inputs[i + 1] - sampler.inputs[i]);
                SceneNode *node = channel.target;
                if (!node)
                    continue;

                if (channel.targetType == AnimationTargetType::translation) {
                    node->transform.setPosition(
                        glm::mix(sampler.outputs[i], sampler.outputs[i + 1], step)
                    );
                }

                if (channel.targetType == AnimationTargetType::rotation) {
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

                    node->transform.setRotation(glm::normalize(glm::slerp(q1, q2, step)));
                }

                if (channel.targetType == AnimationTargetType::scale) {
                    node->transform.setScale(
                        glm::mix(sampler.outputs[i], sampler.outputs[i + 1], step)
                    );
                }
            }
        }
    }

    for (auto &node : nodes) {
        updateJoints(node);
    }
}

void Scene::updateJoints( SceneNode &node)
{
    if (node.skin > -1) {
        glm::mat4 inverseTransform = glm::inverse(node.transform.getModelMatrix());
        Skin &skin = skins[node.skin];

        size_t jointsCount = skin.joints.size();
        std::vector<glm::mat4> jointMatrices(jointsCount);

        for (size_t i = 0; i < jointsCount; i++) {
            if (!skin.joints[i]) continue;

            mat4 joint = skin.joints[i]->transform.getModelMatrix();
            jointMatrices[i] = joint * skin.inverseBindMatrices[i];
            jointMatrices[i] = inverseTransform * jointMatrices[i];
        }

        // Update ssbo
        graphics->uploadBuffer(
            skin.jointMatricesBuffer, jointMatrices.data(), jointMatrices.size() * sizeof(glm::mat4)
        );
    }

    for (auto &child : node.children) {
        updateJoints(child);
    }
}

} // namespace rebirth