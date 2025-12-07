#pragma once

#include "math/defines.h"

class vec2
{
public:
    real_t values[2];

    explicit vec2();
    vec2(float val);
    vec2(float newx, float newy);
    inline vec2(const vec2 &vec) = default;

    float       &operator[](const int index);
    const float &operator[](const int index) const;
    vec2        &operator=(float param);
    vec2        &operator=(const vec2 &param);

    vec2 operator+(const vec2 &param);
    vec2 operator-(const vec2 &param);
    vec2 operator*(const vec2 &param);
    vec2 operator/(const vec2 &param);
    vec2 operator+(float param);
    vec2 operator-(float param);
    vec2 operator*(float param);
    vec2 operator/(float param);

    inline const float &x() const { return values[0]; }
    inline const float &y() const { return values[1]; }
    inline const float &r() const { return values[0]; }
    inline const float &g() const { return values[1]; }

    inline float *operator*() { return values; }
};

vec2 operator+(const vec2 &param1, const vec2 &param2);
vec2 operator-(const vec2 &param1, const vec2 &param2);
vec2 operator*(const vec2 &param1, const vec2 &param2);
vec2 operator/(const vec2 &param1, const vec2 &param2);
vec2 operator+(const vec2 &param1, float param2);
vec2 operator-(const vec2 &param1, float param2);
vec2 operator*(const vec2 &param1, float param2);
vec2 operator/(const vec2 &param1, float param2);

real_t length(const vec2 &param);
float dot(const vec2 &param1, const vec2 &param2);
vec2 normalize(const vec2 &param);
float distance(const vec2 &param1, const vec2 &param2);