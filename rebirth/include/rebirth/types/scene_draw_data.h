#pragma once

#include <rebirth/math/math.h>
#include <rebirth/types/id_types.h>

struct SceneDrawData
{
    mat4 projection;
    mat4 view;
    vec4 cameraPosAndLightNum;
    ImageID shadowMapId = ImageID::Invalid;
};