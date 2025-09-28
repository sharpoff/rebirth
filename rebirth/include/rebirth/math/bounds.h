#pragma once

#include <rebirth/math/math.h>
#include <rebirth/types/id_types.h>
#include <rebirth/math/transform.h>
#include <rebirth/types/mesh.h>

struct Vertex;
struct Model;

struct Bounds
{
    vec3 origin = vec3(0.0f);
    float sphereRadius = 0.0f;
    vec3 extents = vec3(0.0f);
};

Bounds calculateBoundingBox(std::vector<Vertex> &vertices);
Bounds calculateBoundingBox(Model &model, Transform transform);

Bounds calculateBoundingSphere(std::vector<Vertex> &vertices);
Bounds calculateBoundingSphere(Mesh &mesh);