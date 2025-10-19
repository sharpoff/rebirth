#pragma once

#include "EASTL/string.h"
#include "EASTL/unordered_map.h"
#include "EASTL/vector.h"

#include "game/game_object.h"

class GameWorld
{
public:
    void initialize();
    void shutdown();

private:
    eastl::vector<GameObject>                   gameObjects;
    eastl::unordered_map<eastl::string, size_t> gameObjectsMap;
};