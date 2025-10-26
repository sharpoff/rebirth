#include "game/world.h"

#include "core/resource_manager.h"
#include "math/bounds.h"
#include "physics/physics.h"

#include "util/logger.h"

#include <Jolt/Physics/Body/BodyID.h>

void World::initialize(Physics *physics)
{
    assert(physics);

    int32_t cubeMeshId = ResourceManager::get()->getMeshIndexByName("Cube");

    if (cubeMeshId > -1) {
        Entity floor(physics);
        floor.setName("Floor");
        floor.setScale(vec3(10.0f, 0.1f, 10.0f));
        floor.setBounds(math::calculateBoundingBox(cubeMeshId, floor.getTransform()));
        floor.setMesh(cubeMeshId);
        floor.setStatic(true);
        floor.setBody(physics->createBox(floor.getPosition(), floor.getRotation(), floor.getBounds().extents, floor.isStatic()));
        floor.setOverrideMaterial(ResourceManager::get()->getMaterialIndexByName("green"));
        entities.push_back(floor);

        for (int i = -2; i <= 2; i++) {
            Entity box(physics);
            box.setName("Box " + eastl::to_string(i));
            box.setPosition(vec3(i * 2.3f, 20.0f, 0.0f));
            box.setBounds(math::calculateBoundingBox(cubeMeshId, box.getTransform()));
            box.setMesh(cubeMeshId);
            box.setStatic(false);
            box.setBody(physics->createBox(box.getPosition(), box.getRotation(), box.getBounds().extents, box.isStatic()));
            box.setOverrideMaterial(ResourceManager::get()->getMaterialIndexByName("red"));
            entities.push_back(box);
        }
    }

    logger::logInfo("World initialized");
}

void World::shutdown()
{
    logger::logInfo("World shutdown");
}

void World::update(float deltaTime)
{
    // Update entities transforms
    for (auto &entity : entities) {
        entity.update(deltaTime);
    }
}

Entity *World::getEntityByName(eastl::string name)
{
    // XXX: not optimized
    for (auto &entity : entities) {
        if (entity.getName() == name) {
            return &entity;
        }
    }

    return nullptr;
}

Entity *World::getEntityByIndex(size_t index)
{
    if (index >= 0 && index < entities.size()) {
        return &entities[index];
    }

    return nullptr;
}

Entity *World::getEntityByBodyId(uint32_t bodyId)
{
    // XXX: not optimized
    for (auto &entity : entities) {
        if (entity.getBodyID() == JPH::BodyID(bodyId)) {
            return &entity;
        }
    }

    return nullptr;
}