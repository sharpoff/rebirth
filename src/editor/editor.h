#pragma once

#include "editor/gizmo.h"
#include "game/entity.h"
#include "core/engine_stats.h"
#include "input/input.h"

#include <imgui.h>

class Editor
{
public:
    void initialize(EngineStats *engineStats);

    void update(Input *input, Camera *camera, const vec2 &screenDim);
    const eastl::vector<MeshDraw> getGizmoMeshDraws();
    void drawEditor();
    void selectEntity(Entity *entity);

private:
    bool showDemo = false;
    bool showDebug = false;

    Gizmo gizmo;
    Entity *selectedEntity = nullptr;

    EngineStats *engineStats;
};