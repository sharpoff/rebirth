#version 450

#extension GL_GOOGLE_include_directive : require

#include "types.glsl"

#include "scene_data.glsl"
#include "vertices.glsl"
#include "joints.glsl"

layout (push_constant) uniform PushConstant
{
    mat4 transform;
} pc;

void main()
{
    Vertex vertex = vertices[gl_VertexIndex];

    mat4 skinMat = mat4(0.0);
    if (vertex.jointIndices.x > -1) {
        skinMat += vertex.jointWeights.x * jointMatrices[vertex.jointIndices.x];
    }
    if (vertex.jointIndices.y > -1) {
        skinMat += vertex.jointWeights.y * jointMatrices[vertex.jointIndices.y];
    }
    if (vertex.jointIndices.z > -1) {
        skinMat += vertex.jointWeights.z * jointMatrices[vertex.jointIndices.z];
    }
    if (vertex.jointIndices.w > -1) {
        skinMat += vertex.jointWeights.w * jointMatrices[vertex.jointIndices.w];
    }

    if (skinMat == mat4(0.0)) {
        skinMat = mat4(1.0);
    }

    gl_Position = pc.transform * skinMat * vec4(vertex.position, 1.0);
}