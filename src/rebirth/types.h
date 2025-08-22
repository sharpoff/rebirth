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
};

struct Mesh
{
    mat4 matrix = mat4(1.0);
    int materialIdx = -1;
    size_t indexCount = 0;

    vulkan::Buffer vertexBuffer;
    vulkan::Buffer indexBuffer;
};

struct Scene
{
    Transform transform;
    std::vector<Mesh> meshes;
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

struct GPUMeshPushConstant
{
    mat4 worldMatrix;
    VkDeviceAddress address;
    int materialIdx;
};

struct GlobalUBO
{
    mat4 projection;
    mat4 view;
    vec4 cameraPosAndLightNum;
};

enum class LightType
{
    Directional,
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

} // namespace rebirth