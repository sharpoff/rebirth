#pragma once

#include <rebirth/math/math.h>

namespace rebirth
{

struct Material
{
    int baseColorIdx = -1;
    int metallicRoughnessIdx = -1;
    int normalIdx = -1;
    int emissiveIdx = -1;

    vec4 baseColorFactor = vec4(1.0);
    float metallicFactor = 1.0;
    float roughnessFactor = 1.0;

    float _pad0;
    float _pad1;
};

} // namespace rebirth