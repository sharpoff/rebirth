#pragma once

#include <rebirth/math/math.h>
#include <stdint.h>
#include <volk.h>

using MeshID = int32_t;
using MaterialID = int32_t;
using ImageID = int32_t;
using LightID = int32_t;
using RigidBodyID = int32_t;

struct SceneDrawData
{
    mat4 projection;
    mat4 view;
    vec4 cameraPosAndLightNum;
    int shadowMapIndex = -1;
};