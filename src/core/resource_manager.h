#pragma once

#include "EASTL/unordered_map.h"
#include "EASTL/string.h"
#include "EASTL/vector.h"

#include "core/light.h"
#include "core/material.h"
#include "core/mesh.h"
#include "core/model.h"
#include "core/vertex.h"
#include "graphics/vulkan/resources.h"

class ResourceManager
{
public:
    ResourceManager(ResourceManager &) = delete;
    void operator=(ResourceManager &) = delete;

    static ResourceManager *get() {
        static ResourceManager singleton;
        return &singleton;
    }

    int32_t addModel(Model &model, eastl::string name = "");
    int32_t addMesh(Mesh &mesh, eastl::string name = "");
    int32_t addImage(vulkan::Image &image, eastl::string name = "");
    int32_t addMaterial(GPUMaterial &material, eastl::string name = "");
    int32_t addLight(GPULight &light);

    // return offset
    size_t addVertices(const eastl::vector<Vertex> &vertices);

    // return offset
    size_t addIndices(const eastl::vector<uint32_t> &indices);

    Model *getModelByName(eastl::string name);
    Model *getModelByIndex(int32_t index);
    int32_t getModelIndexByName(eastl::string name);
    int32_t getModelIndex(Model &model);

    Mesh *getMeshByName(eastl::string name);
    Mesh *getMeshByIndex(int32_t index);
    int32_t getMeshIndexByName(eastl::string name);
    int32_t getMeshIndex(Mesh &mesh);

    vulkan::Image *getImageByName(eastl::string name);
    vulkan::Image *getImageByIndex(int32_t index);
    int32_t getImageIndexByName(eastl::string name);
    int32_t getImageIndex(vulkan::Image &image);

    GPUMaterial *getMaterialByName(eastl::string name);
    GPUMaterial *getMaterialByIndex(int32_t index);
    int32_t getMaterialIndexByName(eastl::string name);
    int32_t getMaterialIndex(GPUMaterial &material);

    GPULight *getLightByIndex(int32_t index);

    eastl::vector<Model> &getModels() { return models; };
    eastl::vector<Mesh> &getMeshes() { return meshes; };
    eastl::vector<vulkan::Image> &getImages() { return images; };
    eastl::vector<GPUMaterial> &getMaterials() { return materials; };
    eastl::vector<GPULight> &getLights() { return lights; };
    eastl::vector<Vertex> &getVertices() { return vertices; };
    eastl::vector<uint32_t> &getIndices() { return indices; };

    size_t getModelsSize() const { return models.size(); }
    size_t getMeshesSize() const { return meshes.size(); }
    size_t getImagesSize() const { return images.size(); }
    size_t getMaterialsSize() const { return materials.size(); }
    size_t getLightsSize() const { return lights.size(); }
    size_t getVerticesSize() const { return vertices.size(); }
    size_t getIndicesSize() const { return indices.size(); }

private:
    ResourceManager() {};
    ~ResourceManager() {};

    eastl::vector<Model> models;
    eastl::unordered_map<eastl::string, int32_t> modelsMap_;

    eastl::vector<Mesh> meshes;
    eastl::unordered_map<eastl::string, int32_t> meshesMap_;

    eastl::vector<vulkan::Image> images;
    eastl::unordered_map<eastl::string, int32_t> imagesMap_;

    eastl::vector<GPUMaterial> materials;
    eastl::unordered_map<eastl::string, int32_t> materialsMap_;

    eastl::vector<GPULight> lights;

    eastl::vector<Vertex> vertices;
    eastl::vector<uint32_t> indices;
};