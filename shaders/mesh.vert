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

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec4 outTangent;
layout (location = 4) out mat3 outTBN;

void main()
{
    Vertex vertex = pushConstant.vertexBuffer.vertices[gl_VertexIndex];

    vec4 worldPos = pushConstant.worldMatrix * vec4(vertex.position, 1.0);
    gl_Position = ubo.projection * ubo.view * worldPos;

    outWorldPos = vec3(worldPos);
    outUV = vec2(vertex.uv_x, vertex.uv_y);
    outNormal = normalize(mat3(pushConstant.worldMatrix) * vertex.normal);

    outTangent = vertex.tangent;

    vec3 T = normalize(vec3(pushConstant.worldMatrix * vertex.tangent));
    vec3 N = outNormal;
    vec3 B = cross(N, T) * vertex.tangent.w;
    outTBN = mat3(T, B, N);
}
