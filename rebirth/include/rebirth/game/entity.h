#pragma once

#include <rebirth/types/id_types.h>

class Renderer;

enum class EntityType
{
    Undefined,
    Box,
    Sphere,
    Car,
};

struct Entity
{
    Entity() = default;
    virtual ~Entity() = default;

    virtual void update(float deltaTime) {};
    virtual void draw(Renderer &renderer) {};
    virtual void processInput() {};

    const RigidBodyID getRigidBodyId() const { return rigidBodyId; };

protected:
    EntityType entityType = EntityType::Undefined;
    RigidBodyID rigidBodyId = RigidBodyID::Invalid;
};