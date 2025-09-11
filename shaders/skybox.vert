#version 450

#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require

#include "types.glsl"
#include "scene_data.glsl"

layout (buffer_reference, std430) readonly buffer VertexBuffer { Vertex vertices[]; };

layout (push_constant) uniform PushConstant
{
    VertexBuffer vertexBuffer;
    int skyboxId;
} pc;

layout (location = 0) out vec3 outUVW;

void main()
{
    Vertex vertex = pc.vertexBuffer.vertices[gl_VertexIndex];

    outUVW = vertex.position;

    mat4 view = mat4(mat3(scene_data.view)); // remove translation from view
    gl_Position = scene_data.projection * view * vec4(vertex.position, 1.0);
}