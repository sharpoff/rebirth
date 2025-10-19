#pragma once

#include "core/engine_stats.h"

class Editor
{
public:
    void update(EngineStats *engineStats);

    bool showDemo = false;
    bool showDebug = false;
};