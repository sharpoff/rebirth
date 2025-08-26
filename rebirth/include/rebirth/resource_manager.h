#pragma once

#include <rebirth/animation.h>
#include <rebirth/types.h>
#include <rebirth/vulkan/resources.h>

#include <vector>

using namespace rebirth::vulkan;

using ImageIdx = size_t;

namespace rebirth::vulkan
{
class Graphics;
}

namespace rebirth
{

class ResourceManager
{
public:
    void destroy(Graphics &graphics);

    size_t addImage(Image &image);
    size_t addMaterial(Material &material);
    size_t addMesh(GPUMesh &mesh);

    Image &getImage(size_t idx) { return images[idx]; };
    Material &getMaterial(size_t idx) { return materials[idx]; };
    GPUMesh &getMesh(size_t idx) { return meshes[idx]; };

    size_t getImagesCount() { return images.size(); };
    size_t getMaterialsCount() { return materials.size(); };
    size_t getMeshesCount() { return meshes.size(); };

    std::vector<Image> &getImages() { return images; };
    std::vector<Material> &getMaterials() { return materials; };
    std::vector<GPUMesh> &getMeshes() { return meshes; };

private:
    std::vector<Image> images;
    std::vector<Material> materials;
    std::vector<GPUMesh> meshes;
};

} // namespace rebirth