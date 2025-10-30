#pragma once

#include "EASTL/string.h"
#include "EASTL/vector.h"

#include "SDL3/SDL_events.h"
#include "core/application_info.h"

#include "game/entity.h"
#include "game/player.h"

class Physics;
class Camera;

class World
{
public:
    void initialize(Physics *physics, Input *input, const ApplicationInfo &appInfo);
    void shutdown();

    void update(float deltaTime);
    void processEvent(const SDL_Event &event);
    void processInput(float deltaTime);

    Entity *getEntityByName(eastl::string name);
    Entity *getEntityByIndex(size_t index);
    Entity *getEntityByBodyId(uint32_t bodyId);

    eastl::vector<Entity> &getEntities() { return entities; };
    Player &getPlayer() { return player; }

private:
    eastl::vector<Entity> entities;
    Player player;

    Input   *input = nullptr;
    Physics *physics = nullptr;
};