#pragma once

#include "EASTL/string.h"
#include "EASTL/unordered_map.h"
#include "EASTL/vector.h"

#include "game/entity.h"

class Physics;

class World
{
public:
    void initialize(Physics &physics);
    void shutdown();
    void update(float deltaTime, Physics &physics);

    void    addEntity(const Entity &entity, eastl::string name = "");
    Entity *getEntityByName(eastl::string name);
    Entity *getEntityByIndex(uint32_t index);
    Entity *getEntityByBodyId(uint32_t bodyId);

    eastl::vector<Entity> &getEntities() { return entities; };

private:
    eastl::vector<Entity>                         entities;
    eastl::unordered_map<eastl::string, Entity *> entitiesMap;
};