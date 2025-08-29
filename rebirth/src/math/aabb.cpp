#include <rebirth/math/aabb.h>
#include <rebirth/types/mesh.h>

namespace rebirth
{

AABB calculateAABB(std::vector<Vertex> vertices)
{
    vec3 min = vec3(std::numeric_limits<float>::max());
    vec3 max = vec3(std::numeric_limits<float>::min());

    for (auto &vert : vertices) {
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

} // namespace rebirth