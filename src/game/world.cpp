#include "game/world.h"

#include "core/resource_manager.h"
#include "game/entity.h"
#include "math/bounds.h"
#include "math/math.h"
#include "physics/helpers.h"
#include "physics/physics.h"

#include <Jolt/Physics/Body/BodyID.h>

void World::initialize(Physics *physics, Input *input, const ApplicationInfo &appInfo)
{
    assert(physics && input);
    this->physics = physics;
    this->input = input;

    // Entities
    {
        int32_t cubeMeshId = ResourceManager::get()->getMeshIndexByName("Cube");

        // Floor
        {
            vec3 scale = vec3(10.0f, 0.1f, 10.0f);
            Bounds bounds = math::calculateBoundingBox(cubeMeshId, glm::scale(scale));
            bool isStatic = true;
            JPH::BodyID id = physics->createBox(JPH::Vec3::sZero(), JPH::Quat::sIdentity(), MathToJolt(bounds.extents), isStatic);

            EntityCreateInfo createInfo = {
                .name = "Floor",
                .bounds = bounds,
                .scale = scale,
                .bodyId = id,
                .isStatic = isStatic,
            };

            Entity &floor = entities.emplace_back();
            floor.initialize(physics, createInfo);
            floor.setMesh(cubeMeshId);
            floor.setOverrideMaterial(ResourceManager::get()->getMaterialIndexByName("checkerboard"));
        }

        // Boxes
        for (int i = -2; i <= 2; i++) {
            vec3 position = vec3(i * 2.3f, 20.0f, 0.0f);
            Bounds bounds = math::calculateBoundingBox(cubeMeshId, glm::translate(position));
            bool isStatic = false;
            JPH::BodyID id = physics->createBox(MathToJolt(position), JPH::Quat::sIdentity(), MathToJolt(bounds.extents), isStatic);

            EntityCreateInfo createInfo = {
                .name = "Box " + eastl::to_string(i+2),
                .bounds = bounds,
                .bodyId = id,
                .isStatic = isStatic,
            };

            Entity &box = entities.emplace_back();
            box.initialize(physics, createInfo);
            box.setMesh(cubeMeshId);
            box.setOverrideMaterial(ResourceManager::get()->getMaterialIndexByName("checkerboard"));
        }
    }

    player.initialize(physics, input, appInfo);
}

void World::shutdown()
{
    for (auto &entity : entities)
        physics->removeBodyById(entity.getBodyID());
}

void World::update(float deltaTime)
{
    player.update(deltaTime);
}

void World::processEvent(const SDL_Event &event)
{
    player.getCamera().processEvent(event);
}

void World::processInput(float deltaTime)
{
    player.processInput(deltaTime);
}

Entity *World::getEntityByName(eastl::string name)
{
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
    for (auto &entity : entities) {
        if (entity.getBodyID() == JPH::BodyID(bodyId)) {
            return &entity;
        }
    }

    return nullptr;
}