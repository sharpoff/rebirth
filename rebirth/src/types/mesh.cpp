#include <rebirth/types/mesh.h>
#include <rebirth/graphics/vulkan/graphics.h>

#include <rebirth/resource_manager.h>

GPUMesh::GPUMesh(CPUMeshID cpuMeshId, MaterialID materialId)
    : materialId(materialId), cpuMeshId(cpuMeshId)
{
    CPUMesh &cpuMesh = g_resourceManager.getCPUMesh(cpuMeshId);

    vulkan::BufferCreateInfo createInfo = {
        .size = cpuMesh.vertices.size() * sizeof(Vertex),
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    };

    // vertex buffer
    g_graphics.createBuffer(vertexBuffer, createInfo);
    g_graphics.uploadBuffer(vertexBuffer, cpuMesh.vertices.data(), createInfo.size);

    // index buffer
    createInfo.size = cpuMesh.indices.size() * sizeof(uint32_t);
    createInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    g_graphics.createBuffer(indexBuffer, createInfo);
    g_graphics.uploadBuffer(indexBuffer, cpuMesh.indices.data(), createInfo.size);
}