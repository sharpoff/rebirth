#pragma once

#include <rebirth/math.h>
#include <rebirth/transform.h>
#include <rebirth/vulkan/resources.h>

namespace rebirth
{

struct Vertex
{
    vec3 position;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 tangent;
    vec4 jointIndices;
    vec4 jointWeights;
};

struct GPUMesh
{
    vulkan::Buffer vertexBuffer;
    vulkan::Buffer indexBuffer;

    int materialIdx = -1;
    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;
};

struct Material
{
    int baseColorIdx = -1;
    int metallicRoughnessIdx = -1;
    int normalIdx = -1;
    int emissiveIdx = -1;

    vec4 baseColorFactor = vec4(1.0);
    float metallicFactor = 1.0;
    float roughnessFactor = 1.0;
    float _pad0;
    float _pad1;
};

struct SceneData
{
    mat4 projection;
    mat4 view;
    vec4 cameraPosAndLightNum;
    VkDeviceAddress lightsBufferAddress;
    VkDeviceAddress materialsBufferAddress;
    int shadowMapIndex = -1;
};

enum class LightType
{
    Directional = 0,
    Point,
    Spot,
};

struct Light
{
    mat4 mvp = mat4(1.0);
    vec3 color = vec3(0.0);
    vec3 position = vec3(0.0);
    vec3 direction = vec3(0.0); // only for directional light
    LightType type = LightType::Directional;
    float cutOff = glm::cos(glm::radians(12.5f)); // only for spot light
};

struct MeshDrawCommand
{
    size_t meshIdx;
    mat4 transform;
    bool castShadows;
};

} // namespace rebirth