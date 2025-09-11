#pragma once

#include <assert.h>
#include <stddef.h>
#include <math.h>

typedef float real;

namespace math
{
    constexpr const double PI = 3.14159265359;
    constexpr const double TAU = 6.28318530717;

    inline real factorial(real x)
    {
        assert(x >= 0 && "factorial defined only for positive numbers");

        real result = 1;
        for (size_t i = 1; i <= x; i++) {
            result *= i;
        }

        return result;
    }

    inline real radians(real x)
    {
        return x * (PI / 180);
    }

    inline real degrees(real x)
    {
        return x * (180 / PI);
    }
} // namespace math
