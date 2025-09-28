#pragma once

#include <rebirth/types/mesh.h>
#include <rebirth/math/bounds.h>

struct MeshDraw
{
    MeshID meshId = MeshID::Invalid;
    mat4 transform = mat4(1.0f);
    Bounds boundingSphere{};

    // TODO:
    // uint32_t jointMatrixIndex = 0;
};