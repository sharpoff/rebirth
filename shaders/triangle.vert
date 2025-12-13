#version 450

layout(location = 0) out vec3 color;

struct Vertex
{
    vec3 position;
    vec3 color;
};

layout (binding = 1) readonly buffer VertexBuffer {
    Vertex vertices[];
};

void main()
{
    Vertex vertex = vertices[gl_VertexIndex];

    color = vertex.color;
    gl_Position = vec4(vertex.position, 1.0);
}