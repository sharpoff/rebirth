#pragma once

#include <rebirth/math/math.h>
#include <rebirth/types/scene.h>

namespace rebirth
{

struct Vertex;

struct AABB
{
    vec3 min = {};
    vec3 max = {};

    vec3 getHalfExtent() { return vec3(max - min) / 2.0f; }
};

AABB calculateAABB(std::vector<Vertex> vertices);
AABB calculateAABB(Scene &scene, Transform transform);

} // namespace rebirth