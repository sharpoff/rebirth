#version 450

layout (location = 0) in vec3 inUVW;
layout (location = 0) out vec4 fragColor;

layout (binding = 5) uniform samplerCube skybox;

void main()
{
    fragColor = texture(skybox, inUVW);
}