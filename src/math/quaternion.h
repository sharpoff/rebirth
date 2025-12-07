#pragma once

#include "math/defines.h"
#include "math/vec3.h"

class quat
{
public:
    real_t values[4];

    explicit quat();
    quat(float val);
    quat(float newx, float newy, float newz, float neww);
    quat(const vec3 &vec, float val);
    quat(const quat &vec) = default;

    float       &operator[](const int index);
    const float &operator[](const int index) const;
    quat        &operator=(float param);
    quat        &operator=(const quat &param);

    quat operator+(const quat &param);
    quat operator-(const quat &param);
    quat operator*(const quat &param);
    quat operator/(const quat &param);
    quat operator+(float param);
    quat operator-(float param);
    quat operator*(float param);
    quat operator/(float param);

    inline const float &x() const { return values[0]; }
    inline const float &y() const { return values[1]; }
    inline const float &z() const { return values[2]; }
    inline const float &w() const { return values[3]; }

    inline vec2   xy() const { return vec2(values[0], values[1]); }
    inline vec3   xyz() const { return vec3(values[0], values[1], values[2]); }
    inline float *operator*() { return values; }
};