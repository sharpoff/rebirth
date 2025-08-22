#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

// clang-format off
#include <volk.h>
#include <vk_mem_alloc.h>
// clang-format on

#include <array>
#include <assert.h>
#include <filesystem>
#include <vector>

#include <rebirth/vulkan/resources.h>
#include <rebirth/vulkan/swapchain.h>

constexpr int FRAMES_IN_FLIGHT = 2;

namespace rebirth::vulkan
{

class Graphics
{
public:
    Graphics(SDL_Window *window);
    ~Graphics();

private:
    SDL_Window *window{nullptr};

    VkInstance instance{VK_NULL_HANDLE};
    VkSurfaceKHR surface{VK_NULL_HANDLE};

    VkDebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};

    uint32_t graphicsQueueIndex = UINT32_MAX;
    uint32_t presentQueueIndex = UINT32_MAX;
    uint32_t computeQueueIndex = UINT32_MAX;

    VkQueue graphicsQueue{VK_NULL_HANDLE};
    VkQueue presentQueue{VK_NULL_HANDLE};
    VkQueue computeQueue{VK_NULL_HANDLE};

    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    VkDevice device{VK_NULL_HANDLE};

    VmaAllocator allocator{VK_NULL_HANDLE};

    Swapchain swapchain;

    VkCommandPool commandPool{VK_NULL_HANDLE};
    std::array<VkCommandBuffer, FRAMES_IN_FLIGHT> commandBuffers;

    // Sync primitives (per swapchain image)
    std::array<VkSemaphore, FRAMES_IN_FLIGHT> acquireSemaphores;
    std::array<VkFence, FRAMES_IN_FLIGHT> finishRenderFences;
    std::vector<VkSemaphore> submitSemaphores;

    VkDescriptorPool imGuiDesctiptorPool;

    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;

    uint32_t currentFrame = 0;

    bool resizeRequested = false;

private:
    void createInstance();
    void createDebugMessenger();

    void createSurface();
    void createDevice();
    void printDeviceProperties(VkPhysicalDevice physicalDevice);
    void findQueueIndices();

    VkSampleCountFlagBits getMaxSampleCount();

    void createAllocator(VmaAllocatorCreateFlags flags = 0);

    void createCommandPool();
    void createCommandBuffers();

    void createSyncPrimitives();

    void recreateSwapchain();

    void initializeImGui();

public:
    void requestResize() { resizeRequested = true; }

    void uploadBuffer(Buffer &buffer, void *data, VkDeviceSize size);

    VkCommandBuffer beginCommandBuffer();
    void submitCommandBuffer(VkCommandBuffer cmd);

    // Getters
    VmaAllocator &getAllocator() { return allocator; }
    VkDevice &getDevice() { return device; }
    VkPhysicalDevice &getPhysicalDevice() { return physicalDevice; }
    Swapchain &getSwapchain() { return swapchain; }
    VkCommandPool &getCommandPool() { return commandPool; }
    VkSurfaceKHR &getSurface() { return surface; }
    VkQueue &getGraphicsQueue() { return graphicsQueue; }
    VkQueue &getPresentQueue() { return presentQueue; }
    VkQueue &getComputeQueue() { return computeQueue; }
    VkSampleCountFlagBits getSampleCount() const { return sampleCount; }

    // Resources
    void createImage(
        Image *image,
        uint32_t width,
        uint32_t height,
        VkImageUsageFlags usage,
        VkFormat format,
        VkImageViewType viewType,
        VkImageAspectFlags aspect,
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT,
        bool generateMipMaps = false
    );
    void createImageFromFile(Image *image, std::filesystem::path path);
    void createCubemapImage(Image *image, std::filesystem::path dir, VkFormat format);
    void generateMipmaps(Image *image);

    void createBuffer(Buffer *buffer, uint32_t size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

    void destroyImage(Image *image);
    void destroyBuffer(Buffer *buffer);

    VkImageView createImageView(VkImage image, VkImageViewType viewType, VkFormat format, VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
    VkSampler createSampler(VkFilter minFilter, VkFilter magFilter, VkSamplerAddressMode samplerMode, float maxLod);

    // Command buffer
    void flushCommandBuffer(VkCommandBuffer cmd, VkQueue queue, VkCommandPool pool, bool free);
    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool start);

    // Syncronization
    VkSemaphore createSemaphore(VkSemaphoreCreateFlags flags = 0);
    VkFence createFence(VkFenceCreateFlags flags = 0);

    // Descriptor
    VkDescriptorPool createDescriptorPool(std::vector<VkDescriptorPoolSize> poolSizes, VkDescriptorPoolCreateFlags flags = 0);
    VkDescriptorSetLayout createDescriptorSetLayout(VkDescriptorSetLayoutBinding *bindings, uint32_t bindingCount, VkDescriptorBindingFlags *bindingFlags);
    VkDescriptorSet createDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout layout);

    // Pipeline
    VkPipelineLayout createPipelineLayout(VkDescriptorSetLayout *setLayout, VkPushConstantRange *pushConstant);
};

} // namespace rebirth::vulkan
