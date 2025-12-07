#include "math/vec2.h"

vec2::vec2()
{
    values[0] = 0.0f;
    values[1] = 0.0f;
}

vec2::vec2(float val)
{
    values[0] = val;
    values[1] = val;
}

vec2::vec2(float newx, float newy)
{
    values[0] = newx;
    values[1] = newy;
}

float &vec2::operator[](const int index)
{
    return values[index];
}

const float &vec2::operator[](const int index) const
{
    return values[index];
}

vec2 &vec2::operator=(float param)
{
    values[0] = param;
    values[1] = param;
    return *this;
}

vec2 &vec2::operator=(const vec2 &param)
{
    values[0] = param.values[0];
    values[1] = param.values[1];
    return *this;
}

vec2 vec2::operator+(const vec2 &param)
{
    return vec2(values[0] + param.values[0], values[1] + param.values[1]);
}

vec2 vec2::operator-(const vec2 &param)
{
    return vec2(values[0] - param.values[0], values[1] - param.values[1]);
}

vec2 vec2::operator*(const vec2 &param)
{
    return vec2(values[0] * param.values[0], values[1] * param.values[1]);
}

vec2 vec2::operator/(const vec2 &param)
{
    return vec2(values[0] / param.values[0], values[1] / param.values[1]);
}

vec2 vec2::operator+(float param)
{
    return vec2(values[0] + param, values[1] + param);
}

vec2 vec2::operator-(float param)
{
    return vec2(values[0] - param, values[1] - param);
}

vec2 vec2::operator*(float param)
{
    return vec2(values[0] * param, values[1] * param);
}

vec2 vec2::operator/(float param)
{
    return vec2(values[0] / param, values[1] / param);
}

// out class operators

vec2 operator+(const vec2 &param1, const vec2 &param2)
{
    return vec2(param1.values[0] + param2.values[0], param1.values[1] + param2.values[1]);
}

vec2 operator-(const vec2 &param1, const vec2 &param2)
{
    return vec2(param1.values[0] - param2.values[0], param1.values[1] - param2.values[1]);
}

vec2 operator*(const vec2 &param1, const vec2 &param2)
{
    return vec2(param1.values[0] * param2.values[0], param1.values[1] * param2.values[1]);
}

vec2 operator/(const vec2 &param1, const vec2 &param2)
{
    return vec2(param1.values[0] / param2.values[0], param1.values[1] / param2.values[1]);
}

vec2 operator+(const vec2 &param1, float param2)
{
    return vec2(param1.values[0] + param2, param1.values[1] + param2);
}

vec2 operator-(const vec2 &param1, float param2)
{
    return vec2(param1.values[0] - param2, param1.values[1] - param2);
}

vec2 operator*(const vec2 &param1, float param2)
{
    return vec2(param1.values[0] * param2, param1.values[1] * param2);
}

vec2 operator/(const vec2 &param1, float param2)
{
    return vec2(param1.values[0] / param2, param1.values[1] / param2);
}

real_t length(const vec2 &param)
{
    return sqrtf(param[0] * param[0] + param[1] * param[1]);
}

float dot(const vec2 &param1, const vec2 &param2)
{
    return param1[0] * param2[0] + param1[1] * param2[1];
}

vec2 normalize(const vec2 &param)
{
    if (param[0] == 0.0f && param[1] == 0.0f) {
        return vec2(0.0f);
    }

    return param / length(param);
}

float distance(const vec2 &param1, const vec2 &param2)
{
    return length(param1 - param2);
}