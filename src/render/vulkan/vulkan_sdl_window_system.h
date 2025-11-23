#pragma once

#include "SDL3/SDL_video.h"
#include "render/vulkan/vulkan_window_system.h"

class VulkanSDLWindowSystem : public VulkanWindowSystem
{
public:
    VulkanSDLWindowSystem(SDL_Window *window);
    ~VulkanSDLWindowSystem();

    virtual eastl::vector<const char *> getInstanceExtensions() override final;
    virtual bool createSurface(VkInstance instance, VkSurfaceKHR *surface) override final;
    virtual glm::vec2 getWindowSize() override final;

private:
    SDL_Window *pWindow = nullptr;
};