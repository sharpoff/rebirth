#version 450

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "types.glsl"

#include "scene_data.glsl"
#include "textures.glsl"

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 fragColor;

void main()
{
    float depth = 0.0;
    if (scene_data.shadowMapId > -1)
        depth = TEX_2D(scene_data.shadowMapId, inUV).r;

    fragColor = vec4(vec3(depth), 1.0);
}