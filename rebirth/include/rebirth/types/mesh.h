#pragma once

#include <rebirth/graphics/vulkan/resources.h>
#include <rebirth/types/id_types.h>
#include <rebirth/types/vertex.h>
#include <vector>

struct CPUMesh
{
    CPUMesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices) : vertices(vertices), indices(indices) {};

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct GPUMesh
{
    GPUMesh(CPUMeshID cpuMeshId, MaterialID materialId = MaterialID::Invalid);

    vulkan::Buffer vertexBuffer;
    vulkan::Buffer indexBuffer;

    MaterialID materialId = MaterialID::Invalid;
    CPUMeshID cpuMeshId = CPUMeshID::Invalid;
};