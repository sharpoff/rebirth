#pragma once

struct RenderSettings
{
    bool fullscreen = false;
    bool debugShadowMap = false;
    bool wireframe = false;
    bool shadows = true;
    bool skybox = true;
    bool imgui = true;
    float timestampDeltaMs = 0.0f;
    unsigned int drawCount = 0;
};

extern RenderSettings g_renderSettings;