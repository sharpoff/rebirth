#include <rebirth/math/bounds.h>

#include <rebirth/resource_manager.h>
#include <rebirth/types/vertex.h>

Bounds calculateBoundingBox(std::vector<Vertex> &vertices)
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

Bounds calculateBoundingBox(Model &model, Transform transform)
{
    vec3 min = vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    vec3 max = vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

    transform.setPosition(vec3(0, 0, 0));
    transform.setRotation(glm::identity<quat>());

    for (MeshID meshId : model.meshes) {
        Mesh &mesh = g_resourceManager.getMesh(meshId);

        for (size_t i = mesh.indexOffset; i < mesh.indexCount; i++) {
            Vertex vert = g_resourceManager.vertices[g_resourceManager.indices[i]];

            vec4 position = transform.getModelMatrix() * vec4(vert.position.x, vert.position.y, vert.position.z, 1.0f);
            vert.position = vec3(position.x, position.y, position.z);

            min = glm::min(min, vert.position);
            max = glm::max(max, vert.position);
        }
    }

    return Bounds{
        .extents = vec3(max - min) * 0.5f,
    };
}

Bounds calculateBoundingSphere(std::vector<Vertex> &vertices)
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

Bounds calculateBoundingSphere(Mesh &mesh)
{
    vec3 min = vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    vec3 max = vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

    for (size_t i = mesh.indexOffset; i < mesh.indexCount; i++) {
        Vertex vert = g_resourceManager.vertices[g_resourceManager.indices[i]];

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