#pragma once

#include <rebirth/math/math.h>

namespace rebirth
{

enum class LightType
{
    Directional = 0,
    Point,
    Spot,
};

struct Light
{
    mat4 mvp = mat4(1.0);
    vec3 color = vec3(0.0);
    vec3 position = vec3(0.0);
    vec3 direction = vec3(0.0); // only for directional light
    LightType type = LightType::Directional;
    float cutOff = glm::cos(glm::radians(12.5f)); // only for spot light
};

} // namespace rebirth