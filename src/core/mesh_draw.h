#pragma once

#include "math/bounds.h"
#include "core/draw_mask.h"

struct MeshDraw
{
    int32_t meshId = -1;
    int32_t overrideMaterialId = -1;
    uint32_t drawMask = DrawMask::Opaque;
    mat4 transform = mat4(1.0f);
    Bounds boundingSphere{};

    // uint32_t jointMatrixIndex = 0; // TODO
};