#include <rebirth/vulkan/graphics.h>
#include <rebirth/vulkan/swapchain.h>
#include <rebirth/vulkan/util.h>

#include <algorithm>

namespace rebirth::vulkan
{

void Swapchain::initialize(SDL_Window *window, Graphics &graphics)
{
    // Logger::printInfo("Creating swapchain");
    const VkDevice device = graphics.getDevice();
    const VkPhysicalDevice physicalDevice = graphics.getPhysicalDevice();
    const VkSurfaceKHR surface = graphics.getSurface();

    VkSurfaceCapabilitiesKHR capabilities;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics.getPhysicalDevice(), graphics.getSurface(), &capabilities));

    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    extent.width = std::clamp(static_cast<uint32_t>(width), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(static_cast<uint32_t>(height), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    surfaceFormat = getBestSurfaceFormat(physicalDevice, surface);
    presentMode = getBestPresentMode(physicalDevice, surface);

    VkSwapchainCreateInfoKHR swapchainCI{};
    swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCI.surface = graphics.getSurface();
    swapchainCI.minImageCount = capabilities.minImageCount;
    swapchainCI.imageFormat = surfaceFormat.format;
    swapchainCI.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCI.imageExtent = extent;
    swapchainCI.imageArrayLayers = 1;
    swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCI.preTransform = capabilities.currentTransform;
    swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCI.presentMode = presentMode;

    // NOTE: uncomment if VK_SHARING_MODE_CONCURRENT used
    // swapchainCI.queueFamilyIndexCount = 1;
    // swapchainCI.pQueueFamilyIndices = &presentQueueIndex;

    VK_CHECK(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain));

    // get swapchain images
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());

    // create swapchain image views from images
    imageViews.resize(images.size());
    for (uint32_t i = 0; i < imageViews.size(); i++) {
        imageViews[i] = graphics.createImageView(images[i], VK_IMAGE_VIEW_TYPE_2D, surfaceFormat.format);
    }
}

void Swapchain::destroy(VkDevice device) const
{
    // Logger::printInfo("Deleting swapchain");
    for (auto &imageView : imageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

VkResult Swapchain::acquireNextImage(VkDevice device, VkSemaphore &acquireSemaphore) { return vkAcquireNextImageKHR(device, swapchain, ~0ull, acquireSemaphore, nullptr, &imageIndex); }

VkResult Swapchain::present(VkQueue queue, VkSemaphore &submitSemaphore) const
{
    // Present
    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &submitSemaphore;

    return vkQueuePresentKHR(queue, &presentInfo);
}

VkPresentModeKHR Swapchain::getBestPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32_t presentModeCount;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data()));

    for (auto &mode : presentModes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR Swapchain::getBestSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32_t surfaceFormatCount;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr));
    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data()));

    for (auto &format : surfaceFormats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB) {
            return format;
        }
    }

    return surfaceFormats[0];
}

} // namespace rebirth::vulkan