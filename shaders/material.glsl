#ifndef MATERIAL_GLSL
#define MATERIAL_GLSL

struct Material
{
    int diffuseId;
    int metallicRoughnessId;
    int normalId;
    int emissiveId;

    vec4 color;
    vec4 diffuseFactor;
    float metallicFactor;
    float roughnessFactor;
    uint materialFlags;

    float ambient;
};

const uint MATERIAL_FLAG_AMBIENT = 0;
const uint MATERIAL_FLAG_DIFFUSE = 1 << 1;
const uint MATERIAL_FLAG_METALLICROUGHNESS = 1 << 2;
const uint MATERIAL_FLAG_NORMAL = 1 << 3;
const uint MATERIAL_FLAG_EMISSIVE = 1 << 4;
const uint MATERIAL_FLAG_COLOR = 1 << 5;
const uint MATERIAL_FLAG_ALL = MATERIAL_FLAG_AMBIENT | MATERIAL_FLAG_DIFFUSE | MATERIAL_FLAG_METALLICROUGHNESS | MATERIAL_FLAG_NORMAL | MATERIAL_FLAG_EMISSIVE | MATERIAL_FLAG_COLOR;

#endif