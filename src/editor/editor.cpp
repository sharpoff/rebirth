#include "editor/editor.h"
#include "core/globals.h"

#include "imgui.h"

void Editor::update(EngineStats *engineStats)
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

            if (ImGui::MenuItem("Capture frame")) {
                Globals::captureRenderDoc = true;
            }

            if (ImGui::MenuItem("ImGui Demo", NULL, showDemo)) {
                showDemo = !showDemo;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // if (!ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_MenuBar)) {
    //     ImGui::End();
    //     return;
    // }

    // ImGui::End();
}