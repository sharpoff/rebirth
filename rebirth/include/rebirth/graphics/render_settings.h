#pragma once

struct RenderSettings
{
    bool fullscreen = false;
    bool debugShadowMap = false;
    bool wireframe = false;
    bool shadows = true;
    bool skybox = true;
    bool imgui = true;
};

extern RenderSettings g_renderSettings;