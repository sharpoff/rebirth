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
    Bounds calculateBoundingBox(eastl::vector<Vertex> &vertices);

    Bounds calculateBoundingSphere(eastl::vector<Vertex> &vertices);
    Bounds calculateBoundingSphere(int32_t meshId);
} // namespace math