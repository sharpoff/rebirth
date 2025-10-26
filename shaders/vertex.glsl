#ifndef VERTEX_GLSL
#define VERTEX_GLSL

struct Vertex
{
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 tangent;
    ivec4 jointIndices;
    vec4 jointWeights;
};

layout (binding = 5) readonly buffer VertexBuffer {
    Vertex vertices[];
};

#endif