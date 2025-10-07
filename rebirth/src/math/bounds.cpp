#include <rebirth/math/bounds.h>

#include <rebirth/core/vertex.h>
#include <rebirth/core/mesh.h>

namespace math
{
    Bounds calculateBoundingBox(eastl::vector<Vertex> &vertices)
    {
        if (vertices.empty())
            return Bounds{};

        vec3 min = vertices[0].position;
        vec3 max = vertices[0].position;

        for (Vertex vert : vertices) {
            min = glm::min(min, vert.position);
            max = glm::max(max, vert.position);
        }

        return Bounds{
            .extents = vec3(max - min) * 0.5f,
        };
    }

    Bounds calculateBoundingSphere(eastl::vector<Vertex> &vertices)
    {
        vec3 min = vertices[0].position;
        vec3 max = vertices[0].position;

        for (Vertex &vert : vertices) {
            min = glm::min(min, vert.position);
            max = glm::max(max, vert.position);
        }

        vec3 extents = (max - min) / 2.f;

        return Bounds{
            .origin = (max + min) / 2.f,
            .sphereRadius = glm::length(extents),
            .extents = extents,
        };
    }

    Bounds calculateBoundingSphere(Mesh &mesh, eastl::vector<Vertex> &vertices, eastl::vector<uint32_t> &indices)
    {
        vec3 min = vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        vec3 max = vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

        for (Primitive &primitive : mesh.primitives) {
            for (size_t i = primitive.indexOffset; i < primitive.indexCount; i++) {
                Vertex vert = vertices[indices[i]];

                min = glm::min(min, vert.position);
                max = glm::max(max, vert.position);
            }
        }

        vec3 extents = (max - min) / 2.f;

        return Bounds{
            .origin = (max + min) / 2.f,
            .sphereRadius = glm::length(extents),
            .extents = extents,
        };
    }
} // namespace math