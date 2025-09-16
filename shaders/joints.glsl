#ifndef JOINTS_GLSL
#define JOINTS_GLSL

layout (binding = 4) readonly buffer JointMatricesBuffer {
    mat4 jointMatrices[];
};

#endif