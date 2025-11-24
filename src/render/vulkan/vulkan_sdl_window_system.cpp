#include "render/vulkan/vulkan_sdl_window_system.h"

#include <assert.h>
#include "SDL3/SDL_vulkan.h"

VulkanSDLWindowSystem::VulkanSDLWindowSystem(SDL_Window *window)
{
    assert(window);
    pWindow = window;
}

eastl::vector<const char *> VulkanSDLWindowSystem::getInstanceExtensions()
{
    uint32_t extensionCount = 0;
    const char *const *extensionNames = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    assert(extensionNames && extensionCount > 0);

    return eastl::vector<const char *>(extensionNames, extensionNames + extensionCount);
}

bool VulkanSDLWindowSystem::createSurface(VkInstance instance, VkSurfaceKHR *surface)
{
    assert(surface);
    return SDL_Vulkan_CreateSurface(pWindow, instance, nullptr, surface);
}

glm::vec2 VulkanSDLWindowSystem::getWindowSize()
{
    int width, height;
    SDL_GetWindowSize(pWindow, &width, &height);
    return {width, height};
}