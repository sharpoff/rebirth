#pragma once

#include <stdint.h>

struct EngineStats
{
    uint32_t drawCount        = 0;
    float    timestampDeltaMs = 0.0f;
};