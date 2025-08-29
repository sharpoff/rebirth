#pragma once

#include <rebirth/math/math.h>

namespace rebirth
{

struct Vertex;

struct Sphere
{
    Sphere() = default;
    Sphere(vec3 center, float radius) : center(center), radius(radius) {};

    vec3 center = vec3(0.0f);
    float radius = 0.0f;
};

Sphere calculateBoundingSphere(std::vector<Vertex> vertices);

} // namespace rebirth