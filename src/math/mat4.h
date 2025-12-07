#pragma once

#include "math/defines.h"
#include "math/vec4.h"

class mat4
{
public:
    real_t values[16];

    mat4();
    mat4(real_t m00, real_t m01, real_t m02, real_t m03, real_t m10, real_t m11, real_t m12, real_t m13, real_t m20, real_t m21, real_t m22, real_t m23, real_t m30, real_t m31, real_t m32, real_t m33);

    float       &operator[](const int index);
    const float &operator[](const int index) const;
    mat4        &operator=(float param);
    mat4        &operator=(const mat4 &param);

    mat4 operator+(const mat4 &param);
    mat4 operator-(const mat4 &param);
    mat4 operator*(const mat4 &param);
    mat4 operator/(const mat4 &param);

    mat4 operator+(const vec4 &param);
    mat4 operator-(const vec4 &param);
    mat4 operator/(const vec4 &param);

    vec4 operator*(const vec4 &param);

    mat4 operator+(float param);
    mat4 operator-(float param);
    mat4 operator*(float param);
    mat4 operator/(float param);

    inline vec4 column(const int index);
    inline vec4 row(const int index);

    static inline mat4 identity() {
        return mat4(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    };
};