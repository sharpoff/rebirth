#include "math/vec3.h"

vec3::vec3()
{
    values[0] = 0.0f;
    values[1] = 0.0f;
    values[2] = 0.0f;
}

vec3::vec3(float val)
{
    values[0] = val;
    values[1] = val;
    values[2] = val;
}

vec3::vec3(float newx, float newy, float newz)
{
    values[0] = newx;
    values[1] = newy;
    values[2] = newz;
}

vec3::vec3(const vec2 &vec, float val)
{
    values[0] = vec[0];
    values[1] = vec[1];
    values[2] = val;
}

float &vec3::operator[](const int index)
{
    return values[index];
}

const float &vec3::operator[](const int index) const
{
    return values[index];
}

vec3 &vec3::operator=(float param)
{
    values[0] = param;
    values[1] = param;
    values[2] = param;
    values[3] = param;
    return *this;
}

vec3 &vec3::operator=(const vec3 &param)
{
    values[0] = param.values[0];
    values[1] = param.values[1];
    values[2] = param.values[2];
    return *this;
}

vec3 vec3::operator+(const vec3 &param)
{
    return vec3(values[0] + param.values[0], values[1] + param.values[1], values[2] + param.values[2]);
}

vec3 vec3::operator-(const vec3 &param)
{
    return vec3(values[0] - param.values[0], values[1] - param.values[1], values[2] - param.values[2]);
}

vec3 vec3::operator*(const vec3 &param)
{
    return vec3(values[0] * param.values[0], values[1] * param.values[1], values[2] * param.values[2]);
}

vec3 vec3::operator/(const vec3 &param)
{
    return vec3(values[0] / param.values[0], values[1] / param.values[1], values[2] / param.values[2]);
}

vec3 vec3::operator+(float param)
{
    return vec3(values[0] + param, values[1] + param, values[2] + param);
}

vec3 vec3::operator-(float param)
{
    return vec3(values[0] - param, values[1] - param, values[2] - param);
}

vec3 vec3::operator*(float param)
{
    return vec3(values[0] * param, values[1] * param, values[2] * param);
}

vec3 vec3::operator/(float param)
{
    return vec3(values[0] / param, values[1] / param, values[2] / param);
}

// out class operators

vec3 operator+(const vec3 &param1, const vec3 &param2)
{
    return vec3(param1.values[0] + param2.values[0], param1.values[1] + param2.values[1], param1.values[2] + param2.values[2]);
}

vec3 operator-(const vec3 &param1, const vec3 &param2)
{
    return vec3(param1.values[0] - param2.values[0], param1.values[1] - param2.values[1], param1.values[2] - param2.values[2]);
}

vec3 operator*(const vec3 &param1, const vec3 &param2)
{
    return vec3(param1.values[0] * param2.values[0], param1.values[1] * param2.values[1], param1.values[2] * param2.values[2]);
}

vec3 operator/(const vec3 &param1, const vec3 &param2)
{
    return vec3(param1.values[0] / param2.values[0], param1.values[1] / param2.values[1], param1.values[2] / param2.values[2]);
}

vec3 operator+(const vec3 &param1, float param2)
{
    return vec3(param1.values[0] + param2, param1.values[1] + param2, param1.values[2] + param2);
}

vec3 operator-(const vec3 &param1, float param2)
{
    return vec3(param1.values[0] - param2, param1.values[1] - param2, param1.values[2] - param2);
}

vec3 operator*(const vec3 &param1, float param2)
{
    return vec3(param1.values[0] * param2, param1.values[1] * param2, param1.values[2] * param2);
}

vec3 operator/(const vec3 &param1, float param2)
{
    return vec3(param1.values[0] / param2, param1.values[1] / param2, param1.values[2] / param2);
}

real_t length(const vec3 &param)
{
    return sqrtf(param[0] * param[0] + param[1] * param[1] + param[2] * param[2]);
}

float dot(const vec3 &param1, const vec3 &param2)
{
    return param1[0] * param2[0] + param1[1] * param2[1] + param1[2] * param2[2];
}

vec3 cross(const vec3 &param1, const vec3 &param2)
{
    vec3 result;
    result[0] = param1[1] * param2[2] - param1[2] * param2[1];
    result[1] = param1[2] * param2[0] - param1[0] * param2[2];
    result[2] = param1[0] * param2[1] - param1[1] * param2[0];
    return result;
}

vec3 normalize(const vec3 &param)
{
    if (param[0] == 0.0f && param[1] == 0.0f && param[2] == 0.0f) {
        return vec3(0.0f);
    }

    return param / length(param);
}