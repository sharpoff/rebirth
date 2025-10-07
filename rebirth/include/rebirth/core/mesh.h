#pragma once

#include <rebirth/core/vertex.h>
#include <rebirth/math/bounds.h>

struct Primitive
{
    int materialIndex = -1;

    uint32_t indexOffset = 0;
    uint32_t indexCount = 0;
    uint32_t vertexOffset = 0;
    uint32_t vertexCount = 0;
};

struct Mesh
{
    eastl::vector<Primitive> primitives;
};

struct MeshDraw
{
    Mesh &mesh;
    mat4 transform = mat4(1.0f);
    Bounds boundingSphere{};

    // TODO:
    // uint32_t jointMatrixIndex = 0;
};