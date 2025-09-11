#pragma once

#include <rebirth/types/animation.h>
#include <rebirth/types/light.h>
#include <rebirth/types/material.h>
#include <rebirth/types/mesh.h>
#include <rebirth/types/model.h>

#include <rebirth/types/id_types.h>

#include <rebirth/graphics/vulkan/resources.h>

class ResourceManager
{
public:
    ResourceManager() = default;
    ResourceManager(ResourceManager const &) = delete;
    void operator=(ResourceManager const &) = delete;

    void destroy();

    ImageID addImage(vulkan::Image &image);
    MaterialID addMaterial(Material &material);
    ModelID addModel(Model &model);
    GPUMeshID addGPUMesh(GPUMesh &mesh);
    CPUMeshID addCPUMesh(CPUMesh &mesh);
    LightID addLight(Light &light);

    vulkan::Image &getImage(ImageID id) { return images[ID(id)]; }
    Material &getMaterial(MaterialID id) { return materials[ID(id)]; }
    Model &getModel(ModelID id) { return models[ID(id)]; }
    GPUMesh &getGPUMesh(GPUMeshID id) { return gpuMeshes[ID(id)]; }
    CPUMesh &getCPUMesh(CPUMeshID id) { return cpuMeshes[ID(id)]; }
    Light &getLight(LightID id) { return lights[ID(id)]; }

    std::vector<vulkan::Image> images;
    std::vector<Material> materials;
    std::vector<Model> models;
    std::vector<GPUMesh> gpuMeshes;
    std::vector<CPUMesh> cpuMeshes;
    std::vector<Light> lights;
};

extern ResourceManager g_resourceManager;