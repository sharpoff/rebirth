#ifndef TYPES
#define TYPES

struct Vertex
{
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 tangent;
    ivec4 jointIndices;
    vec4 jointWeights;
};

struct Material
{
    int baseColorIdx;
    int metallicRoughnessIdx;
    int normalIdx;
    int emissiveIdx;

    vec4 baseColorFactor;
    float metallicFactor;
    float roughnessFactor;

    float _pad0;
    float _pad1;
};

struct Light
{
    mat4 mvp;
    vec3 position;
    int type;       // enum LightType
    vec3 color;
    float cutOff;   // only for spot light
    vec3 direction; // only for directional light

    float _pad0;
};

const uint LIGHT_TYPE_DIRECTIONAL = 1;
const uint LIGHT_TYPE_POINT = 2;
const uint LIGHT_TYPE_SPOT = 3;

#endif