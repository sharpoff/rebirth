#pragma once

#include <math/math.h>
#include <EASTL/vector.h>

struct Vertex;
struct Mesh;

struct Bounds
{
    vec3 origin = vec3(0.0f);
    float sphereRadius = 0.0f;
    vec3 extents = vec3(0.0f);
};

namespace math
{
    Bounds calculateBoundingBox(Mesh mesh, mat4 transform = mat4(1.0f));
    Bounds calculateBoundingSphere(Mesh mesh, mat4 transform = mat4(1.0f));
} // namespace math