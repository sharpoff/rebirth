#version 450

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "scene_data.glsl"

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 fragColor;

layout (binding = 1) uniform sampler2D textures[];

#define TEX(id, uv) texture(textures[nonuniformEXT(id)], uv)

void main()
{
    float depth = TEX(scene_data.shadowMapIndex, inUV).r;
    fragColor = vec4(vec3(depth), 1.0);
}