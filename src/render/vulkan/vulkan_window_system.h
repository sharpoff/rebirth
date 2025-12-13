#pragma once

#include "core/stl.h"
#include <math/math.h>

#include <volk.h>

class VulkanWindowSystem
{
public:
    virtual ~VulkanWindowSystem() = default;

    virtual Vector<const char *> getInstanceExtensions() = 0;
    virtual bool createSurface(VkInstance instance, VkSurfaceKHR *surface) = 0;
    virtual vec2 getWindowSize() = 0;
};