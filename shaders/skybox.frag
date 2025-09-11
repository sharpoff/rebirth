#version 450

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require

#include "types.glsl"
#include "textures.glsl"

layout (location = 0) in vec3 inUVW;
layout (location = 0) out vec4 fragColor;

layout (buffer_reference, std430) readonly buffer VertexBuffer { Vertex vertices[]; };

layout (push_constant) uniform PushConstant
{
    VertexBuffer vertexBuffer;
    int skyboxId;
} pc;

void main()
{
    fragColor = texture(textureCubes[nonuniformEXT(pc.skyboxId)], inUVW);
}