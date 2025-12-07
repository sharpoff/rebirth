#pragma once

#include "math/defines.h"
#include "math/vec2.h"

class vec3
{
public:
    real_t values[3];

    explicit vec3();
    vec3(float val);
    vec3(float newx, float newy, float newz);
    vec3(const vec2 &vec, float val);
    vec3(const vec3 &vec) = default;

    float       &operator[](const int index);
    const float &operator[](const int index) const;
    vec3        &operator=(float param);
    vec3        &operator=(const vec3 &param);

    vec3 operator+(const vec3 &param);
    vec3 operator-(const vec3 &param);
    vec3 operator*(const vec3 &param);
    vec3 operator/(const vec3 &param);
    vec3 operator+(float param);
    vec3 operator-(float param);
    vec3 operator*(float param);
    vec3 operator/(float param);

    inline const float &x() const { return values[0]; }
    inline const float &y() const { return values[1]; }
    inline const float &z() const { return values[2]; }
    inline const float &r() const { return values[0]; }
    inline const float &g() const { return values[1]; }
    inline const float &b() const { return values[2]; }

    inline vec2   xy() const { return vec2(values[0], values[1]); }
    inline float *operator*() { return values; }
};

vec3 operator+(const vec3 &param1, const vec3 &param2);
vec3 operator-(const vec3 &param1, const vec3 &param2);
vec3 operator*(const vec3 &param1, const vec3 &param2);
vec3 operator/(const vec3 &param1, const vec3 &param2);
vec3 operator+(const vec3 &param1, float param2);
vec3 operator-(const vec3 &param1, float param2);
vec3 operator*(const vec3 &param1, float param2);
vec3 operator/(const vec3 &param1, float param2);

real_t length(const vec3 &param);
float dot(const vec3 &param1, const vec3 &param2);
vec3 cross(const vec3 &param1, const vec3 &param2);
vec3 normalize(const vec3 &param);