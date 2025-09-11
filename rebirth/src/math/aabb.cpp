#include <rebirth/math/aabb.h>
#include <rebirth/types/mesh.h>

#include <rebirth/resource_manager.h>

AABB calculateAABB(std::vector<Vertex> vertices)
{
    vec3 min = vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    vec3 max = vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

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

AABB calculateAABB(CPUMeshID cpuMeshId, Transform transform)
{
    vec3 min = vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    vec3 max = vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

    CPUMesh &mesh = g_resourceManager.getCPUMesh(cpuMeshId);

    for (Vertex vert : mesh.vertices) {
        // apply transform
        transform.setPosition(vec3(0, 0, 0));
        transform.setRotation(glm::identity<quat>());

        vec4 position = transform.getModelMatrix() * vec4(vert.position.x, vert.position.y, vert.position.z, 1.0f);
        vert.position = vec3(position.x, position.y, position.z);

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

AABB calculateAABB(ModelID modelId, Transform transform)
{
    vec3 min = vec3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    vec3 max = vec3(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

    Model &model = g_resourceManager.getModel(modelId);

    for (GPUMeshID gpuMeshId : model.meshes) {
        GPUMesh &gpuMesh = g_resourceManager.getGPUMesh(gpuMeshId);
        CPUMesh &cpuMesh = g_resourceManager.getCPUMesh(gpuMesh.cpuMeshId);

        for (Vertex vert : cpuMesh.vertices) {
            // apply transform
            transform.setPosition(vec3(0, 0, 0));
            transform.setRotation(glm::identity<quat>());

            vec4 position = transform.getModelMatrix() * vec4(vert.position.x, vert.position.y, vert.position.z, 1.0f);
            vert.position = vec3(position.x, position.y, position.z);

            min.x = std::min(min.x, vert.position.x);
            min.y = std::min(min.y, vert.position.y);
            min.z = std::min(min.z, vert.position.z);

            max.x = std::max(max.x, vert.position.x);
            max.y = std::max(max.y, vert.position.y);
            max.z = std::max(max.z, vert.position.z);
        }
    }

    return AABB{
        .min = min,
        .max = max,
    };
};