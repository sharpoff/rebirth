#include "editor/editor.h"

#include "imgui.h"

void Editor::update()
{
    if (!ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save level")) {}
            if (ImGui::MenuItem("Open level")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Place")) {
            if (ImGui::MenuItem("Object")) {}
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    ImGui::End();
}