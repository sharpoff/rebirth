#include "math/quaternion.h"

quat::quat()
{
    values[0] = 0.0f;
    values[1] = 0.0f;
    values[2] = 0.0f;
    values[3] = 1.0f;
}

quat::quat(float val)
{
    values[0] = val;
    values[1] = val;
    values[2] = val;
    values[3] = val;
}

quat::quat(float newx, float newy, float newz, float neww)
{
    values[0] = newx;
    values[1] = newy;
    values[2] = newz;
    values[3] = neww;
}

quat::quat(const vec3 &vec, float val)
{
    values[0] = vec[0];
    values[1] = vec[1];
    values[2] = vec[2];
    values[3] = val;
}

float &quat::operator[](const int index)
{
    return values[index];
}

const float &quat::operator[](const int index) const
{
    return values[index];
}

quat &quat::operator=(float param)
{
    values[0] = param;
    values[1] = param;
    values[2] = param;
    values[3] = param;
    return *this;
}

quat &quat::operator=(const quat &param)
{
    values[0] = param[0];
    values[1] = param[1];
    values[2] = param[2];
    values[3] = param[3];
    return *this;
}