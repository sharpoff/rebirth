#include <rebirth/game/game.h>

#include <rebirth/graphics/renderer.h>

namespace Game
{
    std::vector<Entity*> entities;

    void initialize()
    {
    }

    void shutdown()
    {
        for (Entity *entity : entities) {
            delete entity;
        }
    }

    EntityID addEntity(Entity &entity)
    {
        entities.push_back(&entity);
        return EntityID(entities.size() - 1);
    }

    Entity *getEntityById(EntityID id)
    {
        if (id != EntityID::Invalid && ID(id) < entities.size()) {
            return entities[ID(id)];
        }

        return nullptr;
    }

    void update(float deltaTime)
    {
        for (Entity *entity : entities) {
            entity->update(deltaTime);
        }
    }

    void draw(Renderer &renderer)
    {
        for (Entity *entity : entities) {
            entity->draw(renderer);
        }
    }

    void processInput()
    {
        for (Entity *entity : entities) {
            entity->processInput();
        }
    }
}; // namespace Game