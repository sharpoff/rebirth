#include <rebirth/math/projection.h>

namespace math
{
    mat4 perspective(float fov, float aspectRatio, float near, float far)
    {
        float f = 1.0f / tan(fov * 0.5f);

        // clang-format off
        return mat4(
            f / aspectRatio, 0, 0, 0,
            0, -f, 0, 0,
            0, 0, near / (far - near), -1,
            0, 0, far * near / (far - near), 0
        );
        // clang-format on
    }

    mat4 perspectiveInf(float fov, float aspectRatio, float near)
    {
        float f = 1.0f / tanf(fov * 0.5f);

        // clang-format off
        return mat4(
            f / aspectRatio, 0.0f, 0.0f, 0.0f,
            0.0f, -f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, near, 0.0f
        );
        // clang-format on
    }

    mat4 orthographic(float left, float right, float bottom, float top, float near, float far)
    {
        // clang-format off
        return mat4(
            2.0f / (right - left), 0.0f, 0.0f, -(right + left) / (right - left),
            0.0f, 2.0f / (top - bottom), 0, -(top + bottom) / (top - bottom),
            0, 0, 2.0f / (far - near), -(far + near) / (far - near),
            0, 0, 0, 1
        );
        // clang-format on
    }
} // namespace math