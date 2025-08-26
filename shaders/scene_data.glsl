#ifndef SCENE_DATA
#define SCENE_DATA

#include "types.glsl"

#extension GL_EXT_buffer_reference : require

layout (buffer_reference, std430) readonly buffer MaterialsBuffer { Material materials[]; };
layout (buffer_reference, std430) readonly buffer LightsBuffer { Light lights[]; };

layout (binding = 0) uniform SceneData
{
    mat4 projection;
    mat4 view;
    vec4 cameraPosAndLightNum; // vec4 -> vec3 (camera position) / int (number of lights)
    LightsBuffer lightsBuffer;
    MaterialsBuffer materialsBuffer;
    int shadowMapIndex;
} scene_data;

#endif