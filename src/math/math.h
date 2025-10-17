#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/integer.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

using glm::ivec2;
using glm::ivec3;
using glm::ivec4;
using glm::mat2;
using glm::mat3;
using glm::mat4;
using glm::quat;
using glm::vec2;
using glm::vec3;
using glm::vec4;

namespace math
{
    inline vec3 getPosition(mat4 m)
    {
        vec3 scale;
        quat rotation;
        vec3 position;
        vec3 skew;
        vec4 perspective;

        glm::decompose(m, scale, rotation, position, skew, perspective);

        return position;
    }

    inline quat getRotation(mat4 m)
    {
        vec3 scale;
        quat rotation;
        vec3 position;
        vec3 skew;
        vec4 perspective;

        glm::decompose(m, scale, rotation, position, skew, perspective);

        return rotation;
    }

    inline mat4 perspective(float fov, float aspectRatio, float near, float far)
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

    inline mat4 perspectiveInf(float fov, float aspectRatio, float near)
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

    inline mat4 orthographic(float left, float right, float bottom, float top, float near, float far)
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
