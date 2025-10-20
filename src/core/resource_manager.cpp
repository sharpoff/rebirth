#include "resource_manager.h"
#include "graphics/vulkan/resources.h"

#include "util/logger.h"

int32_t ResourceManager::addMesh(const Mesh &mesh, eastl::string name)
{
    size_t id = meshes.size();
    meshes.push_back(mesh);

    if (!name.empty())
        meshesMap[name] = id;

    return id;
}

int32_t ResourceManager::addImage(const vulkan::Image &image, eastl::string name)
{
    size_t id = images.size();
    images.push_back(image);

    if (!name.empty())
        imagesMap[name] = id;

    return id;
}

int32_t ResourceManager::addMaterial(const GPUMaterial &material, eastl::string name)
{
    size_t id = materials.size();
    materials.push_back(material);

    if (!name.empty())
        materialsMap[name] = id;

    return id;
}

int32_t ResourceManager::addLight(const GPULight &light)
{
    size_t id = lights.size();
    lights.push_back(light);
    return id;
}

Mesh *ResourceManager::getMeshByName(eastl::string name)
{
    if (meshesMap.find(name) != meshesMap.end())
        return &meshes[meshesMap[name]];

    for (auto &[k, v] : meshesMap) {
        std::cout << k.c_str() << " ";
    }
    std::cout << "\n";

    logger::logError("Failed to get mesh by name ", name.c_str());
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
    if (meshesMap.find(name) != meshesMap.end())
        return meshesMap[name];

    return -1;
}

int32_t ResourceManager::getMeshIndex(Mesh *mesh)
{
    return std::distance(meshes.begin(), mesh);
}

vulkan::Image *ResourceManager::getImageByName(eastl::string name)
{
    if (imagesMap.find(name) != imagesMap.end())
        return &images[imagesMap[name]];

    logger::logError("Failed to get image by name ", name.c_str());
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
    if (imagesMap.find(name) != imagesMap.end())
        return imagesMap[name];

    return -1;
}

int32_t ResourceManager::getImageIndex(vulkan::Image *image)
{
    return std::distance(images.begin(), image);
}

GPUMaterial *ResourceManager::getMaterialByName(eastl::string name)
{
    if (materialsMap.find(name) != materialsMap.end())
        return &materials[materialsMap[name]];

    logger::logError("Failed to get material by name ", name.c_str());
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
    if (materialsMap.find(name) != materialsMap.end())
        return materialsMap[name];

    return -1;
}

int32_t ResourceManager::getMaterialIndex(GPUMaterial *material)
{
    return std::distance(materials.begin(), material);
}

GPULight *ResourceManager::getLightByIndex(int32_t index)
{
    if (index >= 0 && index < int(lights.size()))
        return &lights[index];

    return nullptr;
}