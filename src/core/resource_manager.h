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

    eastl::vector<Mesh> &getMeshes() { return m_meshes; };
    eastl::vector<vulkan::Image> &getImages() { return m_images; };
    eastl::vector<GPUMaterial> &getMaterials() { return m_materials; };
    eastl::vector<GPULight> &getLights() { return m_lights; };
    eastl::vector<Vertex> &getVertices() { return m_vertices; };
    eastl::vector<uint32_t> &getIndices() { return m_indices; };

    size_t getMeshesSize() const { return m_meshes.size(); }
    size_t getImagesSize() const { return m_images.size(); }
    size_t getMaterialsSize() const { return m_materials.size(); }
    size_t getLightsSize() const { return m_lights.size(); }
    size_t getVerticesSize() const { return m_vertices.size(); }
    size_t getIndicesSize() const { return m_indices.size(); }

private:
    ResourceManager() {};
    ~ResourceManager() {};

    eastl::vector<Mesh> m_meshes;
    eastl::unordered_map<eastl::string, int32_t> m_meshesMap;

    eastl::vector<vulkan::Image> m_images;
    eastl::unordered_map<eastl::string, int32_t> m_imagesMap;

    eastl::vector<GPUMaterial> m_materials;
    eastl::unordered_map<eastl::string, int32_t> m_materialsMap;

    eastl::vector<GPULight> m_lights;

    eastl::vector<Vertex> m_vertices;
    eastl::vector<uint32_t> m_indices;
};