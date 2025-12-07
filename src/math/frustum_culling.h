#pragma once

#include "math/bounds.h"
#include "math/mat4.h"

namespace math
{
    bool isSphereVisible(const Bounds &sphere, mat4 viewProj, mat4 transform);
}