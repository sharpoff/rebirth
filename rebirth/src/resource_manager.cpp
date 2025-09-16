#include <rebirth/resource_manager.h>

#include <rebirth/graphics/vulkan/graphics.h>

ResourceManager g_resourceManager;

void ResourceManager::destroy()
{
    for (auto &image : images) {
        if (image.image)
            g_graphics.destroyImage(image);
    }

    if (vertexBuffer.buffer)
        g_graphics.destroyBuffer(vertexBuffer);

    if (indexBuffer.buffer)
        g_graphics.destroyBuffer(indexBuffer);

    if (jointMatricesBuffer.buffer)
        g_graphics.destroyBuffer(jointMatricesBuffer);
}

ImageID ResourceManager::addImage(vulkan::Image &image)
{
    images.push_back(image);
    return ImageID(images.size() - 1);
}

MaterialID ResourceManager::addMaterial(Material &material)
{
    materials.push_back(material);
    return MaterialID(materials.size() - 1);
}

ModelID ResourceManager::addModel(Model &model)
{
    models.push_back(model);
    return ModelID(models.size() - 1);
}

MeshID ResourceManager::addMesh(Mesh &mesh)
{
    meshes.push_back(mesh);
    return MeshID(meshes.size() - 1);
}

LightID ResourceManager::addLight(Light &light)
{
    lights.push_back(light);
    return LightID(lights.size() - 1);
}

uint32_t ResourceManager::addVerticesAndIndices(std::vector<Vertex> &newVertices, std::vector<uint32_t> &newIndices)
{
    uint32_t vertexOffset = vertices.size();
    uint32_t indexOffset = indices.size();

    vertices.insert(vertices.end(), newVertices.begin(), newVertices.end());

    for (auto &index : newIndices) {
        index += vertexOffset;
    }

    indices.insert(indices.end(), newIndices.begin(), newIndices.end());

    return indexOffset;
}

void ResourceManager::createVertexBuffer()
{
    vulkan::BufferCreateInfo createInfo;
    createInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    createInfo.size = vertices.size() * sizeof(Vertex);

    g_graphics.createBuffer(vertexBuffer, createInfo);
    g_graphics.uploadBuffer(vertexBuffer, vertices.data(), createInfo.size);
}

void ResourceManager::createIndexBuffer()
{
    vulkan::BufferCreateInfo createInfo;
    createInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    createInfo.size = indices.size() * sizeof(uint32_t);

    g_graphics.createBuffer(indexBuffer, createInfo);
    g_graphics.uploadBuffer(indexBuffer, indices.data(), createInfo.size);
}

void ResourceManager::createJointMatricesBuffer()
{
    // TODO:
}