#pragma once

#include "EASTL/string.h"
#include "EASTL/unordered_map.h"
#include "EASTL/vector.h"

#include "game/entity.h"

class Physics;
class Input;

class World
{
public:
    World(Physics *physics, Input *input);
    ~World();

    void update(float deltaTime);
    void processInput(float deltaTime);

    Entity *getEntityByName(eastl::string name);
    eastl::vector<Entity *> &getEntities() { return entities; };

private:
    eastl::vector<Entity *> entities;
    eastl::unordered_map<eastl::string, Entity *> entityNameMap;

    Input   *pInput = nullptr;
    Physics *pPhysics = nullptr;
};