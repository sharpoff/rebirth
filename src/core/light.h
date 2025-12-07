#pragma once

#include <math/common.h>

enum class LightType : unsigned int
{
    Directional = 0,
    Point,
    Spot,
};

struct GPULight
{
    mat4 mvp = mat4::identity();
    vec3 position; // for point and spot lights
    LightType type = LightType::Directional;
    vec3 color = vec3(1.0, 1.0, 1.0);
    float cutOff = cos(math::radians(12.5f)); // for spot light
    vec3 direction; // for directional light

    float _pad0;
};