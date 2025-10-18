#pragma once

#include "math/bounds.h"

struct Primitive
{
    int32_t materialIndex = -1;

    uint32_t indexOffset = 0;
    uint32_t indexCount = 0;
    uint32_t vertexOffset = 0;
    uint32_t vertexCount = 0;
};

struct Mesh
{
    mat4 transform = mat4(1.0f);
    eastl::vector<Primitive> primitives;
};

struct MeshDraw
{
    int32_t meshId = -1;
    Bounds boundingSphere{};

    // TODO:
    // uint32_t jointMatrixIndex = 0;
};