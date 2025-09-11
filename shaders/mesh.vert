#version 450

#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require

#include "types.glsl"
#include "scene_data.glsl"
#include "mesh_pc.glsl"

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec4 outTangent;
layout (location = 4) out mat3 outTBN;

#define getJointMatrix(id) pc.jointMatricesBuffer.jointMatrices[(id)]

void main()
{
    Vertex vertex = pc.vertexBuffer.vertices[gl_VertexIndex];

    mat4 skinMat = mat4(0.0);
    if (vertex.jointIndices.x > -1) {
        skinMat += vertex.jointWeights.x * getJointMatrix(vertex.jointIndices.x);
    }
    if (vertex.jointIndices.y > -1) {
        skinMat += vertex.jointWeights.y * getJointMatrix(vertex.jointIndices.y);
    }
    if (vertex.jointIndices.z > -1) {
        skinMat += vertex.jointWeights.z * getJointMatrix(vertex.jointIndices.z);
    }
    if (vertex.jointIndices.w > -1) {
        skinMat += vertex.jointWeights.w * getJointMatrix(vertex.jointIndices.w);
    }

    if (skinMat == mat4(0.0)) {
        skinMat = mat4(1.0);
    }

    vec4 worldPos = pc.transform * skinMat * vec4(vertex.position, 1.0);
    gl_Position = scene_data.projection * scene_data.view * worldPos;

    outWorldPos = vec3(worldPos);
    outUV = vec2(vertex.uv_x, vertex.uv_y);
    outNormal = transpose(inverse(mat3(pc.transform * skinMat))) * vertex.normal;

    outTangent = vertex.tangent;

    vec3 T = normalize(vec3(pc.transform * skinMat * vertex.tangent));
    vec3 N = outNormal;
    vec3 B = cross(N, T) * vertex.tangent.w;
    outTBN = mat3(T, B, N);
}
