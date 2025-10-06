#pragma once

#include <rebirth/math/math.h>

struct Material
{
    int baseColorId = -1;
    int metallicRoughnessId = -1;
    int normalId = -1;
    int emissiveId = -1;

    vec4 baseColorFactor = vec4(1.0, 1.0, 1.0, 1.0);
    float metallicFactor = 0.0;
    float roughnessFactor = 1.0;

    float _pad0;
    float _pad1;
};