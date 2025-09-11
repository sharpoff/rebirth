#ifndef SCENE_DATA
#define SCENE_DATA

#include "types.glsl"

#extension GL_EXT_buffer_reference : require

layout (binding = 0) uniform SceneData
{
    mat4 projection;
    mat4 view;
    vec4 cameraPosAndLightNum; // vec4 -> vec3 (camera position) / int (number of lights)
    int shadowMapId;
} scene_data;

layout (binding = 2) readonly buffer MaterialsBuffer {
    Material materials[];
};

layout (binding = 3) readonly buffer LightsBuffer {
    Light lights[];
};

#endif