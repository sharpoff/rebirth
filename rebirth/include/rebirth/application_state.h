#pragma once

#include <vector>
#include <rebirth/types/scene.h>

struct ApplicationState
{
    bool fullscreen = false;
    bool debugShadowMap = false;
    bool wireframe = false;
    bool shadows = true;
    bool skybox = true;
    bool imgui = true;

    // XXX: idk, i put it here because imgui needs it. but it should belong to app i guess
    std::vector<rebirth::Scene> scenes;
};