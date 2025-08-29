#pragma once

#include <rebirth/math/math.h>

namespace rebirth
{

struct Vertex;

struct AABB
{
    vec3 min = {};
    vec3 max = {};
};

AABB calculateAABB(std::vector<Vertex> vertices);

} // namespace rebirth