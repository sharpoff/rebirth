#pragma once

#include "math/math.h"
#include "EASTL/vector.h"

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