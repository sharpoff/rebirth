#pragma once

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