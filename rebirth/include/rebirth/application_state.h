#pragma once

#include <rebirth/types/scene.h>

struct ApplicationState
{
    bool fullscreen = false;
    bool debugShadowMap = false;
    bool wireframe = false;
    bool shadows = true;
    bool skybox = true;
    bool imgui = true;

    // std::vector<rebirth::Scene> scenes;
};