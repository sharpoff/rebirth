#pragma once

#include <rebirth/types/id_types.h>
#include <rebirth/types/vertex.h>

// XXX: probably we should separate static and dynamic meshes.
//      So static meshes would have global vertex/index buffers and dynamic meshes would have per mesh buffers?
//      Or something like one separate *dynamic* buffer.
//      We should also think about offsets inside meshes, because if we delete dynamic mesh we should correct offsets of all meshes after it.

struct Mesh
{
    Mesh(MaterialID materialId, uint32_t indexOffset, uint32_t indexCount) : materialId(materialId), indexOffset(indexOffset), indexCount(indexCount) {}

    MaterialID materialId = MaterialID::Invalid;

    uint32_t indexOffset = 0;
    uint32_t indexCount = 0;
};