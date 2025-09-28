#version 450

#extension GL_GOOGLE_include_directive : require

#include "types.glsl"

#include "scene_data.glsl"
#include "vertices.glsl"

layout (location = 0) out vec3 outUVW;

void main()
{
    Vertex vertex = vertices[gl_VertexIndex];

    outUVW = vertex.position;

    mat4 view = mat4(mat3(scene_data.view)); // remove translation from view
    vec4 pos = scene_data.projection * view * vec4(vertex.position, 1.0);
    pos.z = 0.0;
    gl_Position = pos;
}