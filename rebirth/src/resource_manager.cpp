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

ImageID ResourceManager::addImage(vulkan::Image &image)
{
    images.push_back(image);
    return images.size() - 1;
}

MaterialID ResourceManager::addMaterial(Material &material)
{
    materials.push_back(material);
    return materials.size() - 1;
}

MeshID ResourceManager::addMesh(Mesh &mesh)
{
    meshes.push_back(mesh);
    return meshes.size() - 1;
}

LightID ResourceManager::addLight(Light &light)
{
    lights.push_back(light);
    return lights.size() - 1;
}

} // namespace rebirth