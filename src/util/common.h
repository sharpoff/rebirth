#pragma once

#include "math/math.h"

#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))

namespace util
{
    vec3 mouseToWorldDirection(vec2 mouseCoords, vec2 screenDim, mat4 cameraView, mat4 cameraProjection);
}