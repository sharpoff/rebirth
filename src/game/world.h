#pragma once

#include "EASTL/string.h"
#include "EASTL/vector.h"

#include "game/entity.h"

class Physics;

class World
{
public:
    void initialize(Physics *physics);
    void shutdown();
    void update(float deltaTime);

    Entity *getEntityByName(eastl::string name);
    Entity *getEntityByIndex(size_t index);
    Entity *getEntityByBodyId(uint32_t bodyId);

    eastl::vector<Entity> &getEntities() { return entities; };

private:
    eastl::vector<Entity>                         entities;
};