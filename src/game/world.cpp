#include "game/world.h"

#include "core/resource_manager.h"
#include "math/bounds.h"
#include "physics/physics.h"

#include "util/logger.h"

#include <Jolt/Physics/Body/BodyID.h>

void World::initialize(Physics &physics)
{
    int32_t cubeMeshId = ResourceManager::get()->getMeshIndexByName("Cube");
    Mesh   *cubeMesh = ResourceManager::get()->getMeshByIndex(cubeMeshId);

    if (cubeMesh) {
        Entity floor{};
        floor.scale = vec3(10.0f, 0.5f, 10.0f);
        floor.bounds = math::calculateBoundingBox(*cubeMesh, floor.getTransform());
        floor.meshId = cubeMeshId;
        floor.isStatic = true;
        floor.bodyId = physics.createBox(floor);
        floor.overrideMaterialId = ResourceManager::get()->getMaterialIndexByName("checkerboard");
        addEntity(floor, "Floor");

        for (int i = -2; i <= 2; i++) {
            Entity box{};
            box.position = vec3(i * 2.3f, 20.0f, 0.0f);
            box.bounds = math::calculateBoundingBox(*cubeMesh, box.getTransform());
            box.meshId = cubeMeshId;
            box.isStatic = false;
            box.bodyId = physics.createBox(box);
            box.overrideMaterialId = ResourceManager::get()->getMaterialIndexByName("checkerboard");
            addEntity(box, "Box " + eastl::to_string(i));
        }
    }

    logger::logInfo("World initialized");
}

void World::shutdown()
{
    logger::logInfo("World shutdown");
}

void World::update(float deltaTime, Physics &physics)
{
    // Update entities transforms
    for (auto &entity : entities) {
        entity.position = physics.getPosition(entity.bodyId);
        entity.rotation = physics.getRotation(entity.bodyId);
    }
}

void World::addEntity(const Entity &entity, eastl::string name)
{
    entities.push_back(entity);

    if (!name.empty()) {
        entitiesMap[name] = &entities.back();
    }
}

Entity *World::getEntityByName(eastl::string name)
{
    if (!name.empty() && entitiesMap.find(name) != entitiesMap.end()) {
        return entitiesMap[name];
    }

    return nullptr;
}

Entity *World::getEntityByIndex(uint32_t index)
{
    if (index >= 0 && index < uint32_t(entities.size())) {
        return &entities[index];
    }

    return nullptr;
}

Entity *World::getEntityByBodyId(uint32_t bodyId)
{
    // XXX: not optimized
    JPH::BodyID id = JPH::BodyID(bodyId);
    for (auto &entity : entities) {
        if (entity.bodyId == id) {
            return &entity;
        }
    }

    return nullptr;
}