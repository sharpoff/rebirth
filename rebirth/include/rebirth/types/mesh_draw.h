#pragma once

#include <rebirth/types/mesh.h>
#include <rebirth/math/sphere.h>

struct MeshDraw
{
    GPUMeshID meshId = GPUMeshID::Invalid;
    mat4 transform = mat4(1.0f);
    SphereBounding boundingSphere = SphereBounding();

    VkDeviceAddress jointMatricesBuffer = 0;
};