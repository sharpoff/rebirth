#include "math/vec4.h"

vec4::vec4()
{
    values[0] = 0.0f;
    values[1] = 0.0f;
    values[2] = 0.0f;
    values[3] = 0.0f;
}

vec4::vec4(float val)
{
    values[0] = val;
    values[1] = val;
    values[2] = val;
    values[3] = val;
}

vec4::vec4(float newx, float newy, float newz, float neww)
{
    values[0] = newx;
    values[1] = newy;
    values[2] = newz;
    values[3] = neww;
}

vec4::vec4(const vec3 &vec, float val)
{
    values[0] = vec[0];
    values[1] = vec[1];
    values[2] = vec[2];
    values[3] = val;
}

float &vec4::operator[](const int index)
{
    return values[index];
}

const float &vec4::operator[](const int index) const
{
    return values[index];
}

vec4 &vec4::operator=(float param)
{
    values[0] = param;
    values[1] = param;
    values[2] = param;
    values[3] = param;
    return *this;
}

vec4 &vec4::operator=(const vec4 &param)
{
    values[0] = param[0];
    values[1] = param[1];
    values[2] = param[2];
    values[3] = param[3];
    return *this;
}

vec4 vec4::operator+(const vec4 &param)
{
    return vec4(values[0] + param[0], values[1] + param[1], values[2] + param[2], values[3] + param[3]);
}

vec4 vec4::operator-(const vec4 &param)
{
    return vec4(values[0] - param[0], values[1] - param[1], values[2] - param[2], values[3] - param[3]);
}

vec4 vec4::operator*(const vec4 &param)
{
    return vec4(values[0] * param[0], values[1] * param[1], values[2] * param[2], values[3] * param[3]);
}

vec4 vec4::operator/(const vec4 &param)
{
    return vec4(values[0] / param[0], values[1] / param[1], values[2] / param[2], values[3] / param[3]);
}

vec4 vec4::operator+(float param)
{
    return vec4(values[0] + param, values[1] + param, values[2] + param, values[3] + param);
}

vec4 vec4::operator-(float param)
{
    return vec4(values[0] - param, values[1] - param, values[2] - param, values[3] - param);
}

vec4 vec4::operator*(float param)
{
    return vec4(values[0] * param, values[1] * param, values[2] * param, values[3] * param);
}

vec4 vec4::operator/(float param)
{
    return vec4(values[0] / param, values[1] / param, values[2] / param, values[3] / param);
}

// out class operators

vec4 operator+(const vec4 &param1, const vec4 &param2)
{
    return vec4(param1[0] + param2[0], param1[1] + param2[1], param1[2] + param2[2], param1[3] + param2[3]);
}

vec4 operator-(const vec4 &param1, const vec4 &param2)
{
    return vec4(param1[0] - param2[0], param1[1] - param2[1], param1[2] - param2[2], param1[3] - param2[3]);
}

vec4 operator*(const vec4 &param1, const vec4 &param2)
{
    return vec4(param1[0] * param2[0], param1[1] * param2[1], param1[2] * param2[2], param1[3] * param2[3]);
}

vec4 operator/(const vec4 &param1, const vec4 &param2)
{
    return vec4(param1[0] / param2[0], param1[1] / param2[1], param1[2] / param2[2], param1[3] / param2[3]);
}

vec4 operator+(const vec4 &param1, float param2)
{
    return vec4(param1[0] + param2, param1[1] + param2, param1[2] + param2, param1[3] + param2);
}

vec4 operator-(const vec4 &param1, float param2)
{
    return vec4(param1[0] - param2, param1[1] - param2, param1[2] - param2, param1[3] - param2);
}

vec4 operator*(const vec4 &param1, float param2)
{
    return vec4(param1[0] * param2, param1[1] * param2, param1[2] * param2, param1[3] * param2);
}

vec4 operator/(const vec4 &param1, float param2)
{
    return vec4(param1[0] / param2, param1[1] / param2, param1[2] / param2, param1[3] / param2);
}

real_t length(const vec4 &param)
{
    return sqrtf(param[0] * param[0] + param[1] * param[1] + param[2] * param[2] + param[3] * param[3]);
}

float dot(const vec4 &param1, const vec4 &param2)
{
    return param1[0] * param2[0] + param1[1] * param2[1] + param1[2] * param2[2];
}

vec4 cross(const vec4 &param1, const vec4 &param2)
{
    vec4 result;
    result[0] = param1[1] * param2[2] - param1[2] * param2[1];
    result[1] = param1[2] * param2[0] - param1[0] * param2[2];
    result[2] = param1[0] * param2[1] - param1[1] * param2[0];
    return result;
}

vec4 normalize(const vec4 &param)
{
    if (param[0] == 0.0f && param[1] == 0.0f && param[2] == 0.0f && param[3] == 0.0f) {
        return vec4(0.0f);
    }

    return param / length(param);
}