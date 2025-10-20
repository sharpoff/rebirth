#pragma once

class Physics;
struct EngineStats;

class Editor
{
public:
    void initialize(EngineStats *engineStats, Physics *physics);
    void shutdown();

    void update();

    bool showDemo = false;
    bool showDebug = false;

    EngineStats *engineStats;
    Physics *physics;
};