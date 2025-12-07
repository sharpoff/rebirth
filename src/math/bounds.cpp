#include "math/bounds.h"

/*

#include "core/vertex.h"
#include "core/mesh.h"

namespace math
{
    Bounds calculateBoundingBox(int32_t meshId, vec3 scale, quat rotation)
    {
        Mesh *mesh = ResourceManager::get()->getMeshByIndex(meshId);
        if (!mesh) return Bounds{};

        vec3 min = vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        vec3 max = vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

        auto &vertices = ResourceManager::get()->getVertices();
        auto &indices = ResourceManager::get()->getIndices();

        mat4 transform = glm::toMat4(rotation) * glm::scale(scale);

        for (Primitive &primitive : mesh->primitives) {
            for (size_t i = primitive.indexOffset; i < primitive.indexCount; i++) {
                const Vertex &vert = vertices[indices[i]];

                vec3 pos = transform * vec4(vert.position, 1.0f);
                min = glm::min(min, pos);
                max = glm::max(max, pos);
            }
        }

        return Bounds{
            .extents = vec3(max - min) * 0.5f,
        };
    }

    Bounds calculateBoundingSphere(int32_t meshId, vec3 scale, quat rotation)
    {
        Mesh *mesh = ResourceManager::get()->getMeshByIndex(meshId);
        if (!mesh) return Bounds{};

        vec3 min = vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        vec3 max = vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

        auto &vertices = ResourceManager::get()->getVertices();
        auto &indices = ResourceManager::get()->getIndices();

        mat4 transform = glm::toMat4(rotation) * glm::scale(scale);

        for (Primitive &primitive : mesh->primitives) {
            for (size_t i = primitive.indexOffset; i < primitive.indexCount; i++) {
                const Vertex &vert = vertices[indices[i]];

                vec3 pos = transform * vec4(vert.position, 0.0f);
                min = glm::min(min, pos);
                max = glm::max(max, pos);
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

*/