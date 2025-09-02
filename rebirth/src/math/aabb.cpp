#include <rebirth/math/aabb.h>
#include <rebirth/types/mesh.h>

#include <rebirth/resource_manager.h>

namespace rebirth
{

AABB calculateAABB(std::vector<Vertex> vertices)
{
    vec3 min = vec3(std::numeric_limits<float>::max());
    vec3 max = vec3(std::numeric_limits<float>::min());

    for (Vertex vert : vertices) {
        min.x = std::min(min.x, vert.position.x);
        min.y = std::min(min.y, vert.position.y);
        min.z = std::min(min.z, vert.position.z);

        max.x = std::max(max.x, vert.position.x);
        max.y = std::max(max.y, vert.position.y);
        max.z = std::max(max.z, vert.position.z);
    }

    return AABB{
        .min = min,
        .max = max,
    };
}

AABB calculateAABB(Scene &scene, Transform transform)
{
    vec3 min = vec3(std::numeric_limits<float>::max());
    vec3 max = vec3(std::numeric_limits<float>::min());

    for (Vertex vert : scene.vertices) {
        // apply transform
        transform.setPosition(vec3(0.0));
        transform.setRotation(glm::identity<quat>());
        vert.position = vec3(transform.getModelMatrix() * vec4(vert.position, 1.0f));

        min.x = std::min(min.x, vert.position.x);
        min.y = std::min(min.y, vert.position.y);
        min.z = std::min(min.z, vert.position.z);

        max.x = std::max(max.x, vert.position.x);
        max.y = std::max(max.y, vert.position.y);
        max.z = std::max(max.z, vert.position.z);
    }

    return AABB{
        .min = min,
        .max = max,
    };
};

} // namespace rebirth