#pragma once

#include <assert.h>
#include <math.h>

#include "math/defines.h"

#include "math/vec4.h" // IWYU pragma: keep
#include "math/vec3.h" // IWYU pragma: keep
#include "math/vec2.h" // IWYU pragma: keep
#include "math/mat4.h" // IWYU pragma: keep
#include "math/mat3.h" // IWYU pragma: keep
#include "math/quaternion.h" // IWYU pragma: keep

namespace math
{
    inline real_t factorial(real_t x)
    {
        assert(x >= 0 && "factorial defined only for positive numbers");

        real_t result = 1;
        for (size_t i = 1; i <= x; i++) {
            result *= i;
        }

        return result;
    }

    inline real_t radians(real_t x)
    {
        return x * (MATH_PI / 180);
    }

    inline real_t degrees(real_t x)
    {
        return x * (180 / MATH_PI);
    }

    inline quat angleAxis(real_t angle, vec3 axis)
    {
        // TODO:
    }

    inline mat4 lookAt(vec3 eye, vec3 target, vec3 up)
    {
        // TODO:
    }
} // namespace math
