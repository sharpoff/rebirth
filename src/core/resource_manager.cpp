#include "resource_manager.h"
#include "core/light.h"
#include "core/material.h"
#include "graphics/vulkan/resources.h"

#include "util/logger.h"

// FIXME: if you add resource with the same name old resource is not freed!

int32_t ResourceManager::addModel(Model &model, eastl::string name)
{
    size_t id = meshes.size();
    models.push_back(model);

    if (!name.empty())
        modelsMap_[name] = id;

    return id;
}

int32_t ResourceManager::addMesh(Mesh &mesh, eastl::string name)
{
    size_t id = meshes.size();
    meshes.push_back(mesh);

    if (!name.empty())
        meshesMap_[name] = id;

    return id;
}

int32_t ResourceManager::addImage(vulkan::Image &image, eastl::string name)
{
    size_t id = images.size();
    images.push_back(image);

    if (!name.empty())
        imagesMap_[name] = id;

    return id;
}

int32_t ResourceManager::addMaterial(GPUMaterial &material, eastl::string name)
{
    size_t id = materials.size();
    materials.push_back(material);

    if (!name.empty())
        materialsMap_[name] = id;

    return id;
}

int32_t ResourceManager::addLight(GPULight &light)
{
    size_t id = lights.size();
    lights.push_back(light);
    return id;
}

size_t ResourceManager::addVertices(const eastl::vector<Vertex> &vertices)
{
    size_t offset = this->vertices.size();
    this->vertices.insert(this->vertices.end(), vertices.begin(), vertices.end());
    return offset;
}

size_t ResourceManager::addIndices(const eastl::vector<uint32_t> &indices)
{
    size_t offset = this->indices.size();
    this->indices.insert(this->indices.end(), indices.begin(), indices.end());
    return offset;
}

Model *ResourceManager::getModelByName(eastl::string name)
{
    if (modelsMap_.find(name) != modelsMap_.end())
        return &models[modelsMap_[name]];

    LOGE("Failed to get model by name %s", name.c_str());
    return nullptr;
}

Model *ResourceManager::getModelByIndex(int32_t index)
{
    if (index >= 0 && index < int(models.size()))
        return &models[index];
    return nullptr;
}

int32_t ResourceManager::getModelIndexByName(eastl::string name)
{
    if (modelsMap_.find(name) != modelsMap_.end())
        return modelsMap_[name];

    return -1;
}

int32_t ResourceManager::getModelIndex(Model &model)
{
    return std::distance(models.begin(), &model);
}

Mesh *ResourceManager::getMeshByName(eastl::string name)
{
    if (meshesMap_.find(name) != meshesMap_.end())
        return &meshes[meshesMap_[name]];

    LOGE("Failed to get mesh by name %s", name.c_str());
    return nullptr;
}

Mesh *ResourceManager::getMeshByIndex(int32_t index)
{
    if (index >= 0 && index < int(meshes.size()))
        return &meshes[index];
    return nullptr;
}

int32_t ResourceManager::getMeshIndexByName(eastl::string name)
{
    if (meshesMap_.find(name) != meshesMap_.end())
        return meshesMap_[name];

    return -1;
}

int32_t ResourceManager::getMeshIndex(Mesh &mesh)
{
    return std::distance(meshes.begin(), &mesh);
}

vulkan::Image *ResourceManager::getImageByName(eastl::string name)
{
    if (imagesMap_.find(name) != imagesMap_.end())
        return &images[imagesMap_[name]];

    LOGE("Failed to get image by name %s", name.c_str());
    return nullptr;
}

vulkan::Image *ResourceManager::getImageByIndex(int32_t index)
{
    if (index >= 0 && index < int(images.size()))
        return &images[index];

    return nullptr;
}

int32_t ResourceManager::getImageIndexByName(eastl::string name)
{
    if (imagesMap_.find(name) != imagesMap_.end())
        return imagesMap_[name];

    return -1;
}

int32_t ResourceManager::getImageIndex(vulkan::Image &image)
{
    return std::distance(images.begin(), &image);
}

GPUMaterial *ResourceManager::getMaterialByName(eastl::string name)
{
    if (materialsMap_.find(name) != materialsMap_.end())
        return &materials[materialsMap_[name]];

    LOGE("Failed to get material by name %s", name.c_str());
    return nullptr;
}

GPUMaterial *ResourceManager::getMaterialByIndex(int32_t index)
{
    if (index >= 0 && index < int(materials.size()))
        return &materials[index];

    return nullptr;
}

int32_t ResourceManager::getMaterialIndexByName(eastl::string name)
{
    if (materialsMap_.find(name) != materialsMap_.end())
        return materialsMap_[name];

    return -1;
}

int32_t ResourceManager::getMaterialIndex(GPUMaterial &material)
{
    return std::distance(materials.begin(), &material);
}

GPULight *ResourceManager::getLightByIndex(int32_t index)
{
    if (index >= 0 && index < int(lights.size()))
        return &lights[index];

    return nullptr;
}