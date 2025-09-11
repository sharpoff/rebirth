#version 450

#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require

#include "types.glsl"
#include "scene_data.glsl"
#include "mesh_pc.glsl"

layout (location = 0) out vec4 outColor;

void main()
{
    Vertex vertex = pc.vertexBuffer.vertices[gl_VertexIndex];

    gl_Position = scene_data.projection * scene_data.view * pc.transform * vec4(vertex.position, 1.0);
    outColor = vec4(0.0, 1.0, 0.0, 1.0);
}