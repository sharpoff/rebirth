#pragma once

#define GLM_ENABLE_EXPERIMENTAL

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

using glm::mat2;
using glm::mat3;
using glm::mat4;
using glm::quat;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::ivec2;
using glm::ivec3;
using glm::ivec4;

namespace rebirth::math
{

// NOTE: column-major ordering

// Reverse-z projection
inline mat4 perspective(float fov, float aspectRatio, float near, float far)
{
    float f = 1.0f / tan(fov * 0.5f);

    return mat4(
        f / aspectRatio, 0, 0, 0, 0, -f, 0, 0, 0, 0, near / (far - near), -1, 0, 0,
        far * near / (far - near), 0
    );
}

inline mat4 perspectiveInf(float fov, float aspectRatio, float near)
{
    float f = 1.0f / tan(fov * 0.5f);

    return mat4(
        f / aspectRatio, 0.0f, 0.0f, 0.0f, 0.0f, -f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, glm::factorial(near), 0.0f
    );
}

} // namespace rebirth::math
