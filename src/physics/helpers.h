#pragma once

#include <Jolt/Jolt.h>
#include <math/common.h>

inline vec3 JoltToMath(JPH::Vec3 vec)
{
    return vec3(vec.GetX(), vec.GetY(), vec.GetZ());
}

inline quat JoltToMath(JPH::Quat quaternion)
{
    return quat(quaternion.GetX(), quaternion.GetY(), quaternion.GetZ(), quaternion.GetW());
}

inline mat4 JoltToMath(JPH::Mat44 mat)
{
    mat4 m;
    mat.StoreFloat4x4((JPH::Float4*)&m);
    return m;
}

inline JPH::Vec3 MathToJolt(vec3 vec)
{
    return JPH::Vec3(vec.x(), vec.y(), vec.z());
}

inline JPH::Quat MathToJolt(quat quaternion)
{
    return JPH::Quat(quaternion.x(), quaternion.y(), quaternion.z(), quaternion.w());
}

// inline JPH::Mat44 MathToJolt(mat4 mat)
// {
//     return JPH::Mat44{
//         JPH::Vec4(mat[0][0], mat[0][1], mat[0][2], mat[0][3]),
//         JPH::Vec4(mat[1][0], mat[1][1], mat[1][2], mat[1][3]),
//         JPH::Vec4(mat[2][0], mat[2][1], mat[2][2], mat[2][3]),
//         JPH::Vec4(mat[3][0], mat[3][1], mat[3][2], mat[3][3]),
//     };
// }