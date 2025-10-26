#ifndef MESH_PC_GLSL
#define MESH_PC_GLSL

// Draw masks
const uint DRAW_MASK_OPAQUE = 1 << 0;
const uint DRAW_MASK_TRANSPARENT = 1 << 1;
const uint DRAW_MASK_SHADOW = 1 << 2;
const uint DRAW_MASK_LIGHT = 1 << 3;
const uint DRAW_MASK_WIREFRAME = 1 << 4;
const uint DRAW_MASK_OVERLAY = 1 << 5;

layout (push_constant) uniform PushConstant
{
    mat4 transform;
    int materialId;
    uint drawMask;
} pc;

#endif