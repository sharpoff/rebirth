#include <rebirth/game/entities/sphere.h>

#include <rebirth/physics/physics_system.h>
#include <rebirth/resource_manager.h>

#include <rebirth/graphics/renderer.h>

Sphere::Sphere(ModelID modelId, float radius, Transform transform, bool isStatic)
{
    entityType = EntityType::Sphere;
    this->modelId = modelId;
    scale = transform.getScale();

    this->aabb = calculateAABB(modelId, transform);

    rigidBodyId = g_physicsSystem.createSphere(transform, radius, isStatic);
}

void Sphere::draw(Renderer &renderer)
{
    if (rigidBodyId == RigidBodyID::Invalid)
        return;

    RigidBody &rigidBody = g_physicsSystem.getRigidBody(rigidBodyId);

    Transform transform = rigidBody.getTransform();
    transform.setScale(scale);
    renderer.drawModel(modelId, transform);
}