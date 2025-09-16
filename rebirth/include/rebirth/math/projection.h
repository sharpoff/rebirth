#pragma once

#include <rebirth/math/math.h>

// NOTE: column-major ordering

namespace math
{
    mat4 perspective(float fov, float aspectRatio, float near, float far);
    mat4 perspectiveInf(float fov, float aspectRatio, float near);
    mat4 orthographic();
} // namespace math