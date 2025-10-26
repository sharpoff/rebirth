#ifndef LIGHT_GLSL
#define LIGHT_GLSL

struct Light
{
    mat4 mvp;
    vec3 position;
    uint type;       // enum LightType
    vec3 color;
    float cutOff;   // only for spot light
    vec3 direction; // only for directional light

    float _pad0;
};

const uint LIGHT_TYPE_DIRECTIONAL = 0;
const uint LIGHT_TYPE_POINT = 1;
const uint LIGHT_TYPE_SPOT = 2;

#endif