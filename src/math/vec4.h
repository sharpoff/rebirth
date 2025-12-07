#pragma once

#include "math/defines.h"
#include "math/vec3.h"

class vec4
{
public:
    real_t values[4];

    explicit vec4();
    vec4(float val);
    vec4(float newx, float newy, float newz, float neww);
    vec4(const vec3 &vec, float val);
    vec4(const vec4 &vec) = default;

    float       &operator[](const int index);
    const float &operator[](const int index) const;
    vec4        &operator=(float param);
    vec4        &operator=(const vec4 &param);

    vec4 operator+(const vec4 &param);
    vec4 operator-(const vec4 &param);
    vec4 operator*(const vec4 &param);
    vec4 operator/(const vec4 &param);
    vec4 operator+(float param);
    vec4 operator-(float param);
    vec4 operator*(float param);
    vec4 operator/(float param);

    inline const float &x() const { return values[0]; }
    inline const float &y() const { return values[1]; }
    inline const float &z() const { return values[2]; }
    inline const float &w() const { return values[3]; }
    inline const float &r() const { return values[0]; }
    inline const float &g() const { return values[1]; }
    inline const float &b() const { return values[2]; }
    inline const float &a() const { return values[3]; }

    inline vec2   xy() const { return vec2(values[0], values[1]); }
    inline vec3   xyz() const { return vec3(values[0], values[1], values[2]); }
    inline float *operator*() { return values; }
};

vec4 operator+(const vec4 &param1, const vec4 &param2);
vec4 operator-(const vec4 &param1, const vec4 &param2);
vec4 operator*(const vec4 &param1, const vec4 &param2);
vec4 operator/(const vec4 &param1, const vec4 &param2);
vec4 operator+(const vec4 &param1, float param2);
vec4 operator-(const vec4 &param1, float param2);
vec4 operator*(const vec4 &param1, float param2);
vec4 operator/(const vec4 &param1, float param2);

real_t length(const vec4 &param);
float dot(const vec4 &param1, const vec4 &param2);
vec4 cross(const vec4 &param1, const vec4 &param2);
vec4 normalize(const vec4 &param);