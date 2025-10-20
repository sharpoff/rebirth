#include "editor/editor.h"

#include "core/engine_stats.h"
#include "core/globals.h"
#include "imgui.h"
#include "physics/physics.h"

void Editor::initialize(EngineStats *engineStats, Physics *physics)
{
    this->engineStats = engineStats;
    this->physics = physics;
}

void Editor::shutdown()
{
}

void Editor::update()
{
    //
    // Windows
    //
    if (showDemo)
        ImGui::ShowDemoWindow(&showDemo);

    if (showDebug) {
        ImGui::Begin("Debug", &showDebug);

        ImGui::Text("Frame time: %f ms", engineStats->timestampDeltaMs);
        ImGui::Text("FPS: %d", int(1000.0f / engineStats->timestampDeltaMs));
        ImGui::Text("Draw count: %d", engineStats->drawCount);

        if (Globals::selectedEntity) {
            Entity *entity = Globals::selectedEntity;
            ImGui::Separator();

            if (ImGui::SliderFloat3("Entity position", &entity->position[0], -100.0f, 100.0f)) {
                physics->setPosition(entity->bodyId, entity->position);
            }
        }

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