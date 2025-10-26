#include "resource_manager.h"
#include "graphics/vulkan/resources.h"

#include "util/logger.h"

// FIXME: if you add resource with the same name old resource is not freed!

int32_t ResourceManager::addMesh(const Mesh &mesh, eastl::string name)
{
    size_t id = meshes_.size();
    meshes_.push_back(mesh);

    if (!name.empty())
        meshesMap_[name] = id;

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
    size_t id = meshes_.size();
    Mesh &mesh = meshes_.emplace_back();

    if (!name.empty())
        meshesMap_[name] = id;

    return mesh;
}

int32_t ResourceManager::addImage(const vulkan::Image &image, eastl::string name)
{
    size_t id = images_.size();
    images_.push_back(image);

    if (!name.empty())
        imagesMap_[name] = id;

    return id;
}

vulkan::Image &ResourceManager::createNewImage(eastl::string name)
{
    size_t id = images_.size();
    vulkan::Image &image = images_.emplace_back();

    if (!name.empty())
        imagesMap_[name] = id;

    return image;
}

int32_t ResourceManager::addMaterial(const GPUMaterial &material, eastl::string name)
{
    size_t id = materials_.size();
    materials_.push_back(material);

    if (!name.empty())
        materialsMap_[name] = id;

    return id;
}

GPUMaterial &ResourceManager::createNewMaterial(eastl::string name)
{
    size_t id = materials_.size();
    GPUMaterial &material = materials_.emplace_back();

    if (!name.empty())
        materialsMap_[name] = id;

    return material;
}

int32_t ResourceManager::addLight(const GPULight &light)
{
    size_t id = lights_.size();
    lights_.push_back(light);
    return id;
}

GPULight &ResourceManager::createNewLight()
{
    return lights_.emplace_back();
}

size_t ResourceManager::addVertices(const eastl::vector<Vertex> &vertices)
{
    size_t offset = vertices_.size();
    vertices_.insert(vertices_.end(), vertices.begin(), vertices.end());
    return offset;
}

size_t ResourceManager::addIndices(const eastl::vector<uint32_t> &indices)
{
    size_t offset = indices_.size();
    indices_.insert(indices_.end(), indices.begin(), indices.end());
    return offset;
}

Mesh *ResourceManager::getMeshByName(eastl::string name)
{
    if (meshesMap_.find(name) != meshesMap_.end())
        return &meshes_[meshesMap_[name]];

    for (auto &[k, v] : meshesMap_) {
        std::cout << k.c_str() << " ";
    }
    std::cout << "\n";

    logger::logError("Failed to get mesh by name ", name.c_str());
    return nullptr;
}

Mesh *ResourceManager::getMeshByIndex(int32_t index)
{
    if (index >= 0 && index < int(meshes_.size()))
        return &meshes_[index];
    return nullptr;
}

int32_t ResourceManager::getMeshIndexByName(eastl::string name)
{
    if (meshesMap_.find(name) != meshesMap_.end())
        return meshesMap_[name];

    return -1;
}

int32_t ResourceManager::getMeshIndex(Mesh *mesh)
{
    return std::distance(meshes_.begin(), mesh);
}

vulkan::Image *ResourceManager::getImageByName(eastl::string name)
{
    if (imagesMap_.find(name) != imagesMap_.end())
        return &images_[imagesMap_[name]];

    logger::logError("Failed to get image by name ", name.c_str());
    return nullptr;
}

vulkan::Image *ResourceManager::getImageByIndex(int32_t index)
{
    if (index >= 0 && index < int(images_.size()))
        return &images_[index];

    return nullptr;
}

int32_t ResourceManager::getImageIndexByName(eastl::string name)
{
    if (imagesMap_.find(name) != imagesMap_.end())
        return imagesMap_[name];

    return -1;
}

int32_t ResourceManager::getImageIndex(vulkan::Image *image)
{
    return std::distance(images_.begin(), image);
}

GPUMaterial *ResourceManager::getMaterialByName(eastl::string name)
{
    if (materialsMap_.find(name) != materialsMap_.end())
        return &materials_[materialsMap_[name]];

    logger::logError("Failed to get material by name ", name.c_str());
    return nullptr;
}

GPUMaterial *ResourceManager::getMaterialByIndex(int32_t index)
{
    if (index >= 0 && index < int(materials_.size()))
        return &materials_[index];

    return nullptr;
}

int32_t ResourceManager::getMaterialIndexByName(eastl::string name)
{
    if (materialsMap_.find(name) != materialsMap_.end())
        return materialsMap_[name];

    return -1;
}

int32_t ResourceManager::getMaterialIndex(GPUMaterial *material)
{
    return std::distance(materials_.begin(), material);
}

GPULight *ResourceManager::getLightByIndex(int32_t index)
{
    if (index >= 0 && index < int(lights_.size()))
        return &lights_[index];

    return nullptr;
}