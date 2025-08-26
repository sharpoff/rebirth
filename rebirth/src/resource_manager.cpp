#include <rebirth/resource_manager.h>
#include <rebirth/vulkan/graphics.h>

namespace rebirth
{

void ResourceManager::destroy(Graphics &graphics)
{
    for (auto &image : images) {
        graphics.destroyImage(&image);
    }

    for (auto &mesh : meshes) {
        graphics.destroyBuffer(&mesh.vertexBuffer);
        graphics.destroyBuffer(&mesh.indexBuffer);
    }
}

size_t ResourceManager::addImage(vulkan::Image &image)
{
    size_t idx = images.size();
    images.push_back(image);
    return idx;
}

size_t ResourceManager::addMaterial(Material &material)
{
    size_t idx = materials.size();
    materials.push_back(material);
    return idx;
}

size_t ResourceManager::addMesh(GPUMesh &mesh)
{
    size_t idx = meshes.size();
    meshes.push_back(mesh);
    return idx;
}

} // namespace rebirth