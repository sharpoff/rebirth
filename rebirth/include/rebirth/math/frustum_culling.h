#pragma once

#include <rebirth/math/bounds.h>

namespace math
{
    bool isSphereVisible(const Bounds &sphere, mat4 viewProj, mat4 transform);
}