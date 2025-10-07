#pragma once

#include <rebirth/math/math.h>

#include <EASTL/vector.h>
#include <EASTL/string.h>

enum class AnimationPath
{
    invalid,
    translation,
    rotation,
    scale,
    weights // morph targets
};

struct AnimationSampler;

struct AnimationChannel
{
    int samplerIndex = -1;
    int nodeIndex = -1;
    AnimationPath path;
};

struct AnimationSampler
{
    eastl::vector<float> inputs;
    eastl::vector<vec4> outputs;
};

// NOTE: only linear interpolation supported
struct Animation
{
    eastl::string name;
    eastl::vector<AnimationChannel> channels;
    eastl::vector<AnimationSampler> samplers;
    float start = std::numeric_limits<float>::max();
    float end = std::numeric_limits<float>::min();
    float currentTime = 0.0f;
};