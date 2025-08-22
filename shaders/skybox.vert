#version 450

#extension GL_EXT_buffer_reference : require

struct Vertex
{
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 tangent;
};

layout (buffer_reference, std430) readonly buffer VertexBuffer { Vertex vertices[]; };

layout (push_constant) uniform PushConstant
{
    mat4 worldMatrix;
    VertexBuffer vertexBuffer;
    int materialIdx;
} pushConstant;

layout (binding = 0) uniform UBO
{
    mat4 projection;
    mat4 view;
    vec4 cameraPosAndLightNum; // vec4 -> vec3 (camera position) / int (number of lights)
} ubo;

layout (location = 0) out vec3 outUVW;

void main()
{
    Vertex vertex = pushConstant.vertexBuffer.vertices[gl_VertexIndex];

    outUVW = vertex.position;

    mat4 view = mat4(mat3(ubo.view)); // remove translation from view
    gl_Position = ubo.projection * view * vec4(vertex.position, 1.0);
}