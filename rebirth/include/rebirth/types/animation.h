#pragma once

#include <rebirth/math/math.h>
#include <rebirth/types/id_types.h>

#include <vector>
#include <string>

enum class AnimationPath
{
    invalid,
    translation,
    rotation,
    scale,
    weights // morph targets
};

struct AnimationChannel
{
    SamplerID samplerId = SamplerID::Invalid;
    SceneNodeID nodeId = SceneNodeID::Invalid;
    AnimationPath path;
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