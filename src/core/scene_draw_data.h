#pragma once

#include "math/common.h"

struct SceneDrawData
{
    mat4 projection;
    mat4 view;
    vec4 cameraPosAndLightNum;
    int shadowMapIndex;
};