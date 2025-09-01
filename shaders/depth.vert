#version 450

#extension GL_EXT_buffer_reference : require
#extension GL_GOOGLE_include_directive : require

#include "types.glsl"
#include "scene_data.glsl"

layout (buffer_reference, std430) readonly buffer VertexBuffer { Vertex vertices[]; };
layout (buffer_reference, std430) readonly buffer JointMatricesBuffer { mat4 jointMatrices[]; };

layout (push_constant) uniform PushConstant
{
    mat4 transform;
    VertexBuffer vertexBuffer;
    JointMatricesBuffer jointMatricesBuffer;
} pc;

#define getJointMatrix(idx) pc.jointMatricesBuffer.jointMatrices[(idx)]

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

    gl_Position = pc.transform * skinMat * vec4(vertex.position, 1.0);
}