#pragma once

#include <rebirth/math.h>
#include <rebirth/scene.h>

#include <string>
#include <vector>

namespace rebirth
{

enum class AnimationTargetType
{
    invalid,
    translation,
    rotation,
    scale,
    weights // morph targets
};

struct AnimationChannel
{
    size_t sampler = -1; // idx to a sampler
    SceneNode *target = nullptr;
    AnimationTargetType targetType;
};

struct AnimationSampler
{
    std::vector<float> inputs;
    std::vector<vec4> outputs;
};

// NOTE: only linear interpolation supported
struct Animation
{
    std::string name;
    std::vector<AnimationChannel> channels;
    std::vector<AnimationSampler> samplers;
    float start = std::numeric_limits<float>::max();
    float end = std::numeric_limits<float>::min();
    float currentTime = 0.0f;
};

} // namespace rebirth