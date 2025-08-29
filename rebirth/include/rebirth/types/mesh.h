#pragma once

#include <rebirth/math/math.h>
#include <rebirth/math/sphere.h>
#include <rebirth/types/types.h>
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

struct Mesh
{
    vulkan::Buffer vertexBuffer;
    vulkan::Buffer indexBuffer;
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    MaterialID materialIdx = -1;
};

struct DrawCommand
{
    MeshID meshId;
    mat4 transform;
    Sphere boundingSphere;

    VkDeviceAddress jointMatricesBuffer;
};

} // namespace rebirth