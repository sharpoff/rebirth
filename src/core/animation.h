#pragma once

#include <math/common.h>

#include "core/stl.h"

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
    Vector<float> inputs;
    Vector<vec4> outputs;
};

// NOTE: only linear interpolation supported
struct Animation
{
    String name;
    Vector<AnimationChannel> channels;
    Vector<AnimationSampler> samplers;
    float start = std::numeric_limits<float>::max();
    float end = std::numeric_limits<float>::min();
    float currentTime = 0.0f;
};