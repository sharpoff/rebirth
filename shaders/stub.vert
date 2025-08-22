#version 450

#extension GL_EXT_buffer_reference : require

struct Vertex
{
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
};

layout (buffer_reference, std430) readonly buffer VertexBuffer { Vertex vertices[]; };

layout (push_constant) uniform PushConstant
{
    mat4 worldMatrix;
    VertexBuffer vertexBuffer;
    int materialIdx;
} pushConstant;

void main()
{
    Vertex vertex = pushConstant.vertexBuffer.vertices[gl_VertexIndex];

    gl_Position = pushConstant.worldMatrix * vec4(vertex.position, 1.0);
}
