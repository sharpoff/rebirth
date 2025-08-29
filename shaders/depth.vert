#version 450

#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require

#include "types.glsl"
#include "scene_data.glsl"

layout (buffer_reference, std430) readonly buffer VertexBuffer { Vertex vertices[]; };

layout (push_constant) uniform PushConstant
{
    mat4 transform;
    VertexBuffer vertexBuffer;
} pc;

void main()
{
    Vertex vertex = pc.vertexBuffer.vertices[gl_VertexIndex];

    gl_Position = pc.transform * vec4(vertex.position, 1.0);
}