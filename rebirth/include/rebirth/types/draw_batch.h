#pragma once

#include <rebirth/types/id_types.h>

struct DrawBatch
{
    MeshID meshId = MeshID::Invalid;
    uint32_t first = 0;
    uint32_t count = 0;
};