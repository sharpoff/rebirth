#version 450

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require

#include "types.glsl"

layout (location = 0) in vec3 inUVW;
layout (location = 0) out vec4 fragColor;

layout (buffer_reference, std430) readonly buffer VertexBuffer { Vertex vertices[]; };

layout (push_constant) uniform PushConstant
{
    VertexBuffer vertexBuffer;
    int skyboxIndex;
} pc;

// layout (binding = 1) uniform sampler2D textures[];
layout (binding = 1) uniform samplerCube textureCubes[];

void main()
{
    fragColor = texture(textureCubes[nonuniformEXT(pc.skyboxIndex)], inUVW);
}