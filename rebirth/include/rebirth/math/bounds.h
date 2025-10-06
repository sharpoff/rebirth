#pragma once

#include <rebirth/math/math.h>

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
    Bounds calculateBoundingBox(std::vector<Vertex> &vertices);

    Bounds calculateBoundingSphere(std::vector<Vertex> &vertices);
    Bounds calculateBoundingSphere(Mesh &mesh, std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
} // namespace math