#pragma once

#include "math/bounds.h"

// should match shader
enum DrawMask : uint32_t
{
    Opaque = 1 << 0,
    Translucent = 1 << 1,
    Shadow = 1 << 2,
    Light = 1 << 3,
    Wireframe = 1 << 4,
};

struct MeshDraw
{
    int32_t meshId = -1;
    int32_t overrideMaterialId = -1;
    uint32_t drawMask = DrawMask::Opaque;
    mat4 transform = mat4(1.0f);
    Bounds boundingSphere{};

    // uint32_t jointMatrixIndex = 0; // TODO
};