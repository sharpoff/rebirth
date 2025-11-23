#include "game/world.h"

#include "game/entity.h"

World::World(Physics *physics, Input *input)
{
    pInput = input;
    pPhysics = physics;
}

World::~World()
{
}

void World::update(float deltaTime)
{
}

void World::processInput(float deltaTime)
{
    for (Entity *entity : entities) {
        assert(entity);
        entity->processInput(deltaTime);
    }
}

Entity *World::getEntityByName(eastl::string name)
{
    for (auto &entity : entities) {
        if (entity && entity->getName() == name) {
            return entity;
        }
    }

    return nullptr;
}