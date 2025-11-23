#pragma once

#include "EASTL/vector.h"
#include <vulkan/vulkan.h>

#include <glm/vec2.hpp>

class VulkanWindowSystem
{
public:
    virtual ~VulkanWindowSystem() = default;

    virtual eastl::vector<const char *> getInstanceExtensions() = 0;
    virtual bool createSurface(VkInstance instance, VkSurfaceKHR *surface) = 0;
    virtual glm::vec2 getWindowSize() = 0;
};