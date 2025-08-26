#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <volk.h>

namespace rebirth::vulkan
{

class Graphics;

class Swapchain
{
public:
    void init(SDL_Window *window, Graphics &graphics);
    void destroy(VkDevice device) const;

    VkResult acquireNextImage(VkDevice device, VkSemaphore &acquireSemaphore);
    VkResult present(VkQueue queue, VkSemaphore &submitSemaphore) const;

    VkImage &getImage() { return images[imageIndex]; }
    VkImageView &getImageView() { return imageViews[imageIndex]; }
    VkExtent2D getExtent() const { return extent; }
    uint32_t getImagesCount() const { return images.size(); }
    uint32_t getImageIndex() const { return imageIndex; }
    VkPresentModeKHR getPresentMode() const { return presentMode; }
    VkSurfaceFormatKHR getSurfaceFormat() const { return surfaceFormat; }

private:
    VkPresentModeKHR getBestPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    VkSurfaceFormatKHR getBestSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    VkSwapchainKHR swapchain{VK_NULL_HANDLE};
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    uint32_t imageIndex = 0;
    VkExtent2D extent;

    VkPresentModeKHR presentMode;
    VkSurfaceFormatKHR surfaceFormat;
};

} // namespace rebirth::vulkan
