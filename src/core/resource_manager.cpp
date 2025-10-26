#include "resource_manager.h"
#include "graphics/vulkan/resources.h"

#include "util/logger.h"

// FIXME: if you add resource with the same name old resource is not freed!

int32_t ResourceManager::addMesh(const Mesh &mesh, eastl::string name)
{
    size_t id = m_meshes.size();
    m_meshes.push_back(mesh);

    if (!name.empty())
        m_meshesMap[name] = id;

    return id;
}

int32_t ResourceManager::addMesh(const eastl::vector<Vertex> &vertices, const eastl::vector<uint32_t> &indices, mat4 transform, int32_t materialId, eastl::string name)
{
    // create mesh with 1 primitive
    Primitive prim{};
    prim.vertexCount = vertices.size();
    prim.vertexOffset = addVertices(vertices);
    prim.indexCount = indices.size();
    prim.indexOffset = addIndices(indices);
    prim.materialIndex = materialId;

    Mesh mesh{};
    mesh.primitives = {prim};
    mesh.transform = transform;

    return addMesh(mesh, name);
}

Mesh &ResourceManager::createNewMesh(eastl::string name)
{
    size_t id = m_meshes.size();
    Mesh &mesh = m_meshes.emplace_back();

    if (!name.empty())
        m_meshesMap[name] = id;

    return mesh;
}

int32_t ResourceManager::addImage(const vulkan::Image &image, eastl::string name)
{
    size_t id = m_images.size();
    m_images.push_back(image);

    if (!name.empty())
        m_imagesMap[name] = id;

    return id;
}

vulkan::Image &ResourceManager::createNewImage(eastl::string name)
{
    size_t id = m_images.size();
    vulkan::Image &image = m_images.emplace_back();

    if (!name.empty())
        m_imagesMap[name] = id;

    return image;
}

int32_t ResourceManager::addMaterial(const GPUMaterial &material, eastl::string name)
{
    size_t id = m_materials.size();
    m_materials.push_back(material);

    if (!name.empty())
        m_materialsMap[name] = id;

    return id;
}

GPUMaterial &ResourceManager::createNewMaterial(eastl::string name)
{
    size_t id = m_materials.size();
    GPUMaterial &material = m_materials.emplace_back();

    if (!name.empty())
        m_materialsMap[name] = id;

    return material;
}

int32_t ResourceManager::addLight(const GPULight &light)
{
    size_t id = m_lights.size();
    m_lights.push_back(light);
    return id;
}

GPULight &ResourceManager::createNewLight()
{
    return m_lights.emplace_back();
}

size_t ResourceManager::addVertices(const eastl::vector<Vertex> &vertices)
{
    size_t offset = m_vertices.size();
    m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
    return offset;
}

size_t ResourceManager::addIndices(const eastl::vector<uint32_t> &indices)
{
    size_t offset = m_indices.size();
    m_indices.insert(m_indices.end(), indices.begin(), indices.end());
    return offset;
}

Mesh *ResourceManager::getMeshByName(eastl::string name)
{
    if (m_meshesMap.find(name) != m_meshesMap.end())
        return &m_meshes[m_meshesMap[name]];

    for (auto &[k, v] : m_meshesMap) {
        std::cout << k.c_str() << " ";
    }
    std::cout << "\n";

    logger::logError("Failed to get mesh by name ", name.c_str());
    return nullptr;
}

Mesh *ResourceManager::getMeshByIndex(int32_t index)
{
    if (index >= 0 && index < int(m_meshes.size()))
        return &m_meshes[index];
    return nullptr;
}

int32_t ResourceManager::getMeshIndexByName(eastl::string name)
{
    if (m_meshesMap.find(name) != m_meshesMap.end())
        return m_meshesMap[name];

    return -1;
}

int32_t ResourceManager::getMeshIndex(Mesh *mesh)
{
    return std::distance(m_meshes.begin(), mesh);
}

vulkan::Image *ResourceManager::getImageByName(eastl::string name)
{
    if (m_imagesMap.find(name) != m_imagesMap.end())
        return &m_images[m_imagesMap[name]];

    logger::logError("Failed to get image by name ", name.c_str());
    return nullptr;
}

vulkan::Image *ResourceManager::getImageByIndex(int32_t index)
{
    if (index >= 0 && index < int(m_images.size()))
        return &m_images[index];

    return nullptr;
}

int32_t ResourceManager::getImageIndexByName(eastl::string name)
{
    if (m_imagesMap.find(name) != m_imagesMap.end())
        return m_imagesMap[name];

    return -1;
}

int32_t ResourceManager::getImageIndex(vulkan::Image *image)
{
    return std::distance(m_images.begin(), image);
}

GPUMaterial *ResourceManager::getMaterialByName(eastl::string name)
{
    if (m_materialsMap.find(name) != m_materialsMap.end())
        return &m_materials[m_materialsMap[name]];

    logger::logError("Failed to get material by name ", name.c_str());
    return nullptr;
}

GPUMaterial *ResourceManager::getMaterialByIndex(int32_t index)
{
    if (index >= 0 && index < int(m_materials.size()))
        return &m_materials[index];

    return nullptr;
}

int32_t ResourceManager::getMaterialIndexByName(eastl::string name)
{
    if (m_materialsMap.find(name) != m_materialsMap.end())
        return m_materialsMap[name];

    return -1;
}

int32_t ResourceManager::getMaterialIndex(GPUMaterial *material)
{
    return std::distance(m_materials.begin(), material);
}

GPULight *ResourceManager::getLightByIndex(int32_t index)
{
    if (index >= 0 && index < int(m_lights.size()))
        return &m_lights[index];

    return nullptr;
}