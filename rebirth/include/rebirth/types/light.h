#pragma once

#include <rebirth/math/math.h>

enum class LightType
{
    Directional = 0,
    Point,
    Spot,
};

struct Light
{
    mat4 mvp = mat4(1.0f);
    vec3 position; // for point and spot lights
    LightType type = LightType::Directional;
    vec3 color = vec3(1.0, 1.0, 1.0);
    float cutOff = cos(glm::radians(12.5f)); // for spot light
    vec3 direction; // for directional light

    float _pad0;
};