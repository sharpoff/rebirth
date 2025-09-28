#version 450

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "types.glsl"

#include "textures.glsl"

layout (location = 0) in vec2 inUV;
layout (location = 0) out vec4 fragColor;

layout (push_constant) uniform PushConstant
{
    int textureIndex;
};

void main()
{
    if (textureIndex > -1)
        fragColor = TEX_2D(textureIndex, inUV);
    else
        fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}