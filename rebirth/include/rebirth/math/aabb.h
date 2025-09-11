#pragma once

#include <rebirth/math/math.h>
#include <rebirth/math/transform.h>
#include <rebirth/types/mesh.h>

struct Vertex;

struct AABB
{
    vec3 min{};
    vec3 max{};

    vec3 getHalfExtent() { return vec3(max - min) / 2.0f; }
};

AABB calculateAABB(std::vector<Vertex> vertices);
AABB calculateAABB(CPUMeshID cpuMeshId, Transform transform);
AABB calculateAABB(ModelID modelId, Transform transform);