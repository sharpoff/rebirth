#include "render/vulkan/vulkan_swapchain.h"

#include "render/render_types.h"
#include "render/vulkan/vulkan_helpers.h"
#include "render/vulkan/vulkan_types.h"

#include <algorithm>

void VulkanSwapchain::create(const VulkanSwapchainCreateInfo &params)
{
    VkSurfaceCapabilitiesKHR capabilities;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(params.physicalDevice, params.surface, &capabilities));

    vec2 windowSize = params.pWindowSystem->getWindowSize();
    extent.width = std::clamp(static_cast<uint32_t>(windowSize.x()), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(static_cast<uint32_t>(windowSize.y()), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    surfaceFormat = getBestSurfaceFormat(params.physicalDevice, params.surface);
    presentMode = getBestPresentMode(params.physicalDevice, params.surface);

    VkSwapchainCreateInfoKHR swapchainCI{};
    swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCI.surface = params.surface;
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

    VK_CHECK(vkCreateSwapchainKHR(params.device, &swapchainCI, nullptr, &swapchain));

    // get swapchain images
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(params.device, swapchain, &imageCount, nullptr);

    Vector<VkImage> swapchainImages(imageCount);
    vkGetSwapchainImagesKHR(params.device, swapchain, &imageCount, swapchainImages.data());

    // create swapchain image views from images
    for (uint32_t i = 0; i < swapchainImages.size(); i++) {
        VkImageViewCreateInfo imageViewCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        imageViewCreateInfo.image = swapchainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = surfaceFormat.format;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        SharedPtr<VulkanImage> image = eastl::make_shared<VulkanImage>();
        image->image = swapchainImages[i];
        image->format = IMAGE_FORMAT_B8G8R8A8_SRGB;
        image->usage = IMAGE_USAGE_COLOR_ATTACHMENT;
        image->type = IMAGE_TYPE_2D;
        image->sampleCount = 1;
        image->isSwapchain = true;
        image->width = extent.width;
        image->height = extent.height;

        vkCreateImageView(params.device, &imageViewCreateInfo, nullptr, &image->view);
        images.push_back(image);
    }
}

void VulkanSwapchain::destroy(VkDevice device) const
{
    for (auto &image : images) { // NOTE: swapchain VkImages are freed automatically
        vkDestroyImageView(device, image->view, nullptr);
    }

    vkDestroySwapchainKHR(device, swapchain, nullptr);
}

VkResult VulkanSwapchain::acquireNextImage(VkDevice device, VkSemaphore &acquireSemaphore)
{
    return vkAcquireNextImageKHR(device, swapchain, ~0ull, acquireSemaphore, nullptr, &imageIndex);
}

VkResult VulkanSwapchain::present(VkQueue queue, VkSemaphore &submitSemaphore) const
{
    VkPresentInfoKHR presentInfo = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &submitSemaphore;

    return vkQueuePresentKHR(queue, &presentInfo);
}

SharedPtr<VulkanImage> VulkanSwapchain::getImage()
{
    return images[imageIndex];
}

VkPresentModeKHR VulkanSwapchain::getBestPresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32_t presentModeCount;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
    Vector<VkPresentModeKHR> presentModes(presentModeCount);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data()));

    // TODO: add support for other present modes
    for (auto &mode : presentModes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR VulkanSwapchain::getBestSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32_t surfaceFormatCount;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr));
    Vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data()));

    for (auto &format : surfaceFormats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB) {
            return format;
        }
    }

    return surfaceFormats[0];
}