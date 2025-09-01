#pragma once

#include <rebirth/types/animation.h>
#include <rebirth/types/material.h>
#include <rebirth/types/mesh.h>
#include <rebirth/types/light.h>
#include <rebirth/types/types.h>
#include <rebirth/vulkan/resources.h>

#include <vector>

using namespace rebirth::vulkan;

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

    ImageID addImage(Image &image);
    MaterialID addMaterial(Material &material);
    MeshID addMesh(Mesh &mesh);
    LightID addLight(Light &light);

    Image &getImage(ImageID id) { return images[id]; }
    Material &getMaterial(MaterialID id) { return materials[id]; }
    Mesh &getMesh(MeshID id) { return meshes[id]; }
    Light &getLight(LightID id) { return lights[id]; }

    std::vector<Image> images;
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector<Light> lights;
};

} // namespace rebirth