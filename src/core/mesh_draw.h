#pragma once

#include "math/bounds.h"

enum DrawMask : uint32_t
{
    Opaque = 1 << 0,
    Translucent = 1 << 1,
    Shadow = 1 << 2,
    Wireframe = 1 << 3,
};

struct MeshDraw
{
    int32_t meshId = -1;
    mat4 transform = mat4(1.0f);
    Bounds boundingSphere{};
    uint32_t drawMask = 0;

    // uint32_t jointMatrixIndex = 0; // TODO
};