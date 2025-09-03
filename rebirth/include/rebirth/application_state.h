#pragma once

namespace rebirth
{

struct ApplicationState
{
    bool fullscreen = false;
    bool debugShadowMap = false;
    bool wireframe = false;
    bool shadows = true;
    bool skybox = true;
    bool imgui = true;
};

} // namespace rebirth