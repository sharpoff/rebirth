#pragma once

#include "EASTL/unordered_map.h"
#include "EASTL/string.h"
#include "EASTL/vector.h"

#include "core/light.h"
#include "core/material.h"
#include "core/mesh.h"
#include "core/vertex.h"

struct Vertex;
struct Mesh;
struct GPUMaterial;
struct GPULight;

namespace vulkan {
    struct Image;
}

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
    int32_t addImage(const vulkan::Image &image, eastl::string name = "");
    int32_t addMaterial(const GPUMaterial &material);
    int32_t addLight(const GPULight &light);

    Mesh *getMeshByName(eastl::string name);
    Mesh *getMeshByIndex(int32_t index);
    int32_t getMeshIndexByName(eastl::string name);
    int32_t getMeshIndexByMesh(Mesh *mesh);
    vulkan::Image *getImageByName(eastl::string name);
    vulkan::Image *getImageByIndex(int32_t index);
    int32_t getImageIndexByName(eastl::string name);
    int32_t getImageIndexByImage(vulkan::Image *image);
    GPUMaterial *getMaterialByIndex(int32_t index);
    GPULight *getLightByIndex(int32_t index);

    eastl::vector<Mesh> &getMeshes() { return meshes; };
    eastl::vector<vulkan::Image> &getImages() { return images; };
    eastl::vector<GPUMaterial> &getMaterials() { return materials; };
    eastl::vector<GPULight> &getLights() { return lights; };
    eastl::vector<Vertex> &getVertices() { return vertices; };
    eastl::vector<uint32_t> &getIndices() { return indices; };

private:
    ResourceManager() {};
    ~ResourceManager() {};

    eastl::vector<Mesh> meshes;
    eastl::unordered_map<eastl::string, int32_t> meshesMap;

    eastl::vector<vulkan::Image> images;
    eastl::unordered_map<eastl::string, int32_t> imagesMap;

    eastl::vector<GPUMaterial> materials;
    eastl::vector<GPULight> lights;

    eastl::vector<Vertex> vertices;
    eastl::vector<uint32_t> indices;
};