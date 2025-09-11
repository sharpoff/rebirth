#include <rebirth/resource_manager.h>

#include <rebirth/graphics/vulkan/graphics.h>

ResourceManager g_resourceManager;

void ResourceManager::destroy()
{
    for (auto &image : images) {
        g_graphics.destroyImage(image);
    }

    // NOTE: don't destroy model's meshes, because they destroyed here
    for (auto &mesh : gpuMeshes) {
        g_graphics.destroyBuffer(mesh.vertexBuffer);
        g_graphics.destroyBuffer(mesh.indexBuffer);
    }
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

GPUMeshID ResourceManager::addGPUMesh(GPUMesh &mesh)
{
    gpuMeshes.push_back(mesh);
    return GPUMeshID(gpuMeshes.size() - 1);
}

CPUMeshID ResourceManager::addCPUMesh(CPUMesh &mesh)
{
    cpuMeshes.push_back(mesh);
    return CPUMeshID(cpuMeshes.size() - 1);
}

LightID ResourceManager::addLight(Light &light)
{
    lights.push_back(light);
    return LightID(lights.size() - 1);
}