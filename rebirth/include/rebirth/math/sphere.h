#pragma once

#include <rebirth/math/math.h>
#include <vector>

struct Vertex;

struct SphereBounding
{
    SphereBounding() = default;
    SphereBounding(vec3 center, float radius) : center(center), radius(radius) {};

    vec3 center{};
    float radius = 0.0f;
};

SphereBounding calculateBoundingSphere(std::vector<Vertex> vertices);