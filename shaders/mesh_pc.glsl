#ifndef MESH_PC_GLSL
#define MESH_PC_GLSL

layout (push_constant) uniform PushConstant
{
    mat4 transform;
    int materialId;
} pc;

#endif