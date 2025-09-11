#include <rebirth/game/entities/box.h>

#include <rebirth/physics/physics_system.h>
#include <rebirth/resource_manager.h>

#include <rebirth/graphics/renderer.h>

Box::Box(ModelID modelId, Transform transform, bool isStatic)
{
    entityType = EntityType::Box;
    this->modelId = modelId;
    scale = transform.getScale();

    this->aabb = calculateAABB(modelId, transform);

    rigidBodyId = g_physicsSystem.createBox(transform, aabb.getHalfExtent(), isStatic);
}

void Box::draw(Renderer &renderer)
{
    if (rigidBodyId == RigidBodyID::Invalid)
        return;

    RigidBody &rigidBody = g_physicsSystem.getRigidBody(rigidBodyId);

    Transform transform = rigidBody.getTransform();
    transform.setScale(scale);
    renderer.drawModel(modelId, transform);
}