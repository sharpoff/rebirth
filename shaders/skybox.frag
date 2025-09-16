#version 450

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "types.glsl"
#include "textures.glsl"

layout (location = 0) in vec3 inUVW;
layout (location = 0) out vec4 fragColor;

layout (push_constant) uniform PushConstant {
    int skyboxId;
};

void main()
{
    fragColor = TEX_CUBE(skyboxId, inUVW);
}