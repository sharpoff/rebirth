#pragma once

struct RenderSettings
{
    bool fullscreen = false;
    bool drawWireframe = false;
    bool drawShadows = true;
    bool drawMeshes = true;
    bool drawSkybox = true;
    bool drawImGui = true;
    float timestampDeltaMs = 0.0f;
    unsigned int drawCount = 0;
};

extern RenderSettings g_renderSettings;