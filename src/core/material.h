#pragma once

#include "math/math.h"

enum class MaterialFlags : unsigned int
{
    Ambient = 0,
    Diffuse = 1 << 1,
    MetallicRoughness = 1 << 2,
    Normal = 1 << 3,
    Emissive = 1 << 4,
    Color = 1 << 5,

    All = Ambient | Diffuse | MetallicRoughness | Normal | Emissive | Color,
};

// should match the shader
struct GPUMaterial
{
    int diffuseId = -1;
    int metallicRoughnessId = -1;
    int normalId = -1;
    int emissiveId = -1;

    vec4 color = vec4(1.0f);
    vec4 diffuseFactor = vec4(1.0, 1.0, 1.0, 1.0);
    float metallicFactor = 0.0;
    float roughnessFactor = 1.0;
    unsigned int materialFlags = (unsigned int)(MaterialFlags::All);

    float ambient = 0.05f;
};