#include "editor/editor.h"

#include "math/math.h"

#include <imgui.h>

void Editor::update(Input *input, Camera *camera, const ApplicationInfo &appInfo)
{
    assert(input && camera);

    //
    // Gizmo
    //
    if (selectedEntity) {
        mat4 transform = selectedEntity->getTransform();
        
        gizmo.manipulate(input, camera, vec2(appInfo.width, appInfo.height), transform);
        gizmo.selected = true;
    } else {
        gizmo.selected = false;
    }
}

const eastl::vector<MeshDraw> Editor::getGizmoMeshDraws()
{
    return gizmo.getMeshDraws();
}

void Editor::drawEditor(const EngineStats &engineStats)
{
    //
    // Show windows
    //
    if (showDemo)
        ImGui::ShowDemoWindow(&showDemo);

    if (showDebug) {
        ImGui::Begin("Debug", &showDebug);

        ImGui::Text("Frame time: %f ms", engineStats.timestampDeltaMs);
        ImGui::Text("FPS: %d", int(1000.0f / engineStats.timestampDeltaMs));
        ImGui::Text("Draw count: %d", engineStats.drawCount);

        // if (selectedEntity) {
        //     ImGui::Separator();

        //     vec3 position = selectedEntity->getPosition();
        //     if (ImGui::SliderFloat3("Entity position", glm::value_ptr(position), -100.0f, 100.0f)) {
        //         selectedEntity->transformDirty = true;
        //         // selectedEntity->setPosition(position);
        //     }
        // }

        ImGui::End();
    }

    //
    // Menu Bar
    //
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            ImGui::MenuItem("Save level");
            ImGui::MenuItem("Open level");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Place")) {
            ImGui::MenuItem("Object");
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Debug", NULL, showDebug)) {
                showDebug = !showDebug;
            }

            if (ImGui::MenuItem("ImGui Demo", NULL, showDemo)) {
                showDemo = !showDemo;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

void Editor::selectEntity(Entity *entity)
{
    selectedEntity = entity;
}