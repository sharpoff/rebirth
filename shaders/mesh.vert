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
    int materialIdx;
} pc;

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec4 outTangent;
layout (location = 4) out mat3 outTBN;

void main()
{
    Vertex vertex = pc.vertexBuffer.vertices[gl_VertexIndex];

    vec4 worldPos = pc.transform * vec4(vertex.position, 1.0);

    gl_Position = scene_data.projection * scene_data.view * worldPos;

    outWorldPos = vec3(worldPos);
    outUV = vec2(vertex.uv_x, vertex.uv_y);
    outNormal = normalize(mat3(pc.transform) * vertex.normal);

    outTangent = vertex.tangent;

    vec3 T = normalize(vec3(pc.transform * vertex.tangent));
    vec3 N = outNormal;
    vec3 B = cross(N, T) * vertex.tangent.w;
    outTBN = mat3(T, B, N);
}
