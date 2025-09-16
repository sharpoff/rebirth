#ifndef VERTICES_GLSL
#define VERTICES_GLSL

layout (binding = 5) readonly buffer VertexBuffer {
    Vertex vertices[];
};

#endif