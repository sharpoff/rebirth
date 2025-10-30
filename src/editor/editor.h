#pragma once

#include "editor/gizmo.h"
#include "game/entity.h"
#include "core/engine_stats.h"
#include "input/input.h"
#include "core/application_info.h"

class Editor
{
public:
    void update(Input *input, Camera *camera, const ApplicationInfo &appInfo);
    const eastl::vector<MeshDraw> getGizmoMeshDraws();
    void drawEditor(const EngineStats &engineStats);
    void selectEntity(Entity *entity);

private:
    bool showDemo = false;
    bool showDebug = false;

    Gizmo gizmo;
    Entity *selectedEntity = nullptr;
};