#pragma once

#include <rebirth/game/entities/box.h>
#include <rebirth/game/entities/sphere.h>
#include <rebirth/game/entities/car.h>

class Renderer;

namespace Game
{
    void initialize();
    void shutdown();

    EntityID addEntity(Entity *entity);
    Entity *getEntityById(EntityID id);

    void update(float deltaTime);
    void draw(Renderer &renderer);
    void processInput();

} // namespace Game