#pragma once

#include "EASTL/unordered_map.h"
#include "EASTL/string.h"
#include "EASTL/vector.h"

#include "core/light.h"
#include "core/material.h"
#include "core/mesh.h"
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

    int32_t addMesh(const Mesh &mesh, eastl::string name = "");
    int32_t addMesh(const eastl::vector<Vertex> &vertices, const eastl::vector<uint32_t> &indices, mat4 transform = mat4(1.0f), int32_t materialId = -1, eastl::string name = "");
    Mesh &createNewMesh(eastl::string name);

    int32_t addImage(const vulkan::Image &image, eastl::string name = "");
    vulkan::Image &createNewImage(eastl::string name);

    int32_t addMaterial(const GPUMaterial &material, eastl::string name = "");
    GPUMaterial &createNewMaterial(eastl::string name);

    int32_t addLight(const GPULight &light);
    GPULight &createNewLight();

    // return offset
    size_t addVertices(const eastl::vector<Vertex> &vertices);

    // return offset
    size_t addIndices(const eastl::vector<uint32_t> &indices);

    Mesh *getMeshByName(eastl::string name);
    Mesh *getMeshByIndex(int32_t index);
    int32_t getMeshIndexByName(eastl::string name);
    int32_t getMeshIndex(Mesh *mesh);

    vulkan::Image *getImageByName(eastl::string name);
    vulkan::Image *getImageByIndex(int32_t index);
    int32_t getImageIndexByName(eastl::string name);
    int32_t getImageIndex(vulkan::Image *image);

    GPUMaterial *getMaterialByName(eastl::string name);
    GPUMaterial *getMaterialByIndex(int32_t index);
    int32_t getMaterialIndexByName(eastl::string name);
    int32_t getMaterialIndex(GPUMaterial *material);

    GPULight *getLightByIndex(int32_t index);

    eastl::vector<Mesh> &getMeshes() { return meshes_; };
    eastl::vector<vulkan::Image> &getImages() { return images_; };
    eastl::vector<GPUMaterial> &getMaterials() { return materials_; };
    eastl::vector<GPULight> &getLights() { return lights_; };
    eastl::vector<Vertex> &getVertices() { return vertices_; };
    eastl::vector<uint32_t> &getIndices() { return indices_; };

    size_t getMeshesSize() const { return meshes_.size(); }
    size_t getImagesSize() const { return images_.size(); }
    size_t getMaterialsSize() const { return materials_.size(); }
    size_t getLightsSize() const { return lights_.size(); }
    size_t getVerticesSize() const { return vertices_.size(); }
    size_t getIndicesSize() const { return indices_.size(); }

private:
    ResourceManager() {};
    ~ResourceManager() {};

    eastl::vector<Mesh> meshes_;
    eastl::unordered_map<eastl::string, int32_t> meshesMap_;

    eastl::vector<vulkan::Image> images_;
    eastl::unordered_map<eastl::string, int32_t> imagesMap_;

    eastl::vector<GPUMaterial> materials_;
    eastl::unordered_map<eastl::string, int32_t> materialsMap_;

    eastl::vector<GPULight> lights_;

    eastl::vector<Vertex> vertices_;
    eastl::vector<uint32_t> indices_;
};