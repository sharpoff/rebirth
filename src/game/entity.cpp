#include "game/entity.h"
#include "physics/physics.h"
#include "physics/helpers.h"

void Entity::initialize(Physics *physics, const EntityCreateInfo &createInfo)
{
    assert(physics);
    this->physics = physics;

    name = createInfo.name;
    bounds = createInfo.bounds;
    scale = createInfo.scale;
    bodyId = createInfo.bodyId;
    static_ = createInfo.isStatic;
}

mat4 Entity::getTransform() const
{
    return glm::translate(getPosition()) * glm::toMat4(getRotation()) * glm::scale(scale);
}

vec3 Entity::getPosition() const
{
    return JoltToMath(physics->getPosition(bodyId));
}

quat Entity::getRotation() const
{
    return JoltToMath(physics->getRotation(bodyId));
}

void Entity::setMesh(int32_t meshId)
{
    assert(meshId > -1);
    this->meshId = meshId;
}

void Entity::setOverrideMaterial(int32_t materialId)
{
    assert(materialId > -1);
    this->overrideMaterialId = materialId;
}

void Entity::setPosition(vec3 position)
{
    physics->setPosition(bodyId, MathToJolt(position));
}

void Entity::setRotation(quat rotation)
{
    physics->setRotation(bodyId, MathToJolt(glm::normalize(rotation)));
}

void Entity::setScale(vec3 scale)
{
    this->scale = scale;
    physics->setScale(bodyId, MathToJolt(scale));
}