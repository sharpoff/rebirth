#pragma once

#include "core/engine_stats.h"
#include "editor/gizmo.h"
#include "game/entity.h"
#include "input/input.h"

class Camera;
class Input;

class Editor
{
public:
    Editor(Input *input, Camera *camera);
    ~Editor() = default;

    void update(uint32_t appWidth, uint32_t appHeight);
    void drawEditor(const EngineStats &engineStats);
    void selectEntity(Entity *entity);

    const eastl::vector<MeshDraw> getGizmoMeshDraws();

private:
    bool showDemo = false;
    bool showDebug = false;

    Entity *pSelectedEntity = nullptr;

    Gizmo gizmo;

    Input  *pInput = nullptr;
    Camera *pCamera = nullptr;
};