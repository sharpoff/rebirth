#pragma once

#include "math/math.h"

namespace color
{
    constexpr const vec4 red = vec4(1, 0, 0, 1);
    constexpr const vec4 green = vec4(0, 1, 0, 1);
    constexpr const vec4 blue = vec4(0, 0, 1, 1);
    constexpr const vec4 black = vec4(0, 0, 0, 1);
    constexpr const vec4 white = vec4(1, 1, 1, 1);
    constexpr const vec4 yellow = vec4(1, 1, 0, 1);
    constexpr const vec4 cyan = vec4(0, 1, 1, 1);
    constexpr const vec4 purple = vec4(1, 0, 1, 1);
} // namespace color

namespace common
{
    constexpr const vec3 worldUp = vec3(0, 1, 0);
    constexpr const vec3 worldRight = vec3(1, 0, 0);
    constexpr const vec3 worldForward = vec3(0, 0, -1);
} // namespace common