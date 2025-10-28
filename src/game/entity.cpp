#include "game/entity.h"
#include "physics/physics.h"

Entity::Entity(Physics *physics)
{
    assert(physics);
    this->physics_ = physics;
}

void Entity::update(float deltaTime)
{
    if (bodyId != JPH::BodyID()) {
        if (transformDirty) {
            physics_->setPosition(bodyId, position);
            physics_->setRotation(bodyId, glm::normalize(rotation));
        } else {
            position = physics_->getPosition(bodyId);
            rotation = physics_->getRotation(bodyId);
        }
    }
}

mat4 Entity::getTransform() const
{
    return glm::translate(position) * glm::toMat4(rotation) * glm::scale(scale);
}

void Entity::setMesh(int32_t meshId)
{
    assert(meshId > -1);
    this->meshId = meshId;
}

void Entity::setBody(JPH::BodyID bodyId, bool isStatic)
{
    assert(bodyId != JPH::BodyID());
    this->bodyId = bodyId;
}

void Entity::setName(eastl::string name)
{
    assert(!name.empty());
    this->name = name;
}

void Entity::setBounds(Bounds bounds)
{
    this->bounds = bounds;
}

void Entity::setOverrideMaterial(int32_t materialId)
{
    assert(materialId > -1);
    this->overrideMaterialId = materialId;
}

void Entity::setStatic(bool isStatic)
{
    this->static_ = isStatic;
}

void Entity::setPosition(vec3 position)
{
    this->position = position;
}

void Entity::setRotation(quat rotation)
{
    this->rotation = rotation;
}

void Entity::setScale(vec3 scale)
{
    this->scale = scale;
}