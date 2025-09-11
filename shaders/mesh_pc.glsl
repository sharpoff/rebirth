#ifndef MESH_PC
#define MESH_PC

layout (buffer_reference, std430) readonly buffer VertexBuffer { Vertex vertices[]; };
layout (buffer_reference, std430) readonly buffer JointMatricesBuffer { mat4 jointMatrices[]; };

layout (push_constant) uniform PushConstant
{
    mat4 transform;
    VertexBuffer vertexBuffer;
    JointMatricesBuffer jointMatricesBuffer;
    int materialId;
} pc;

#endif