#pragma once

#include <vector>
#include <array>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

// clang-format off
#include <volk.h>
#include <vk_mem_alloc.h>
// clang-format on

#include <assert.h>
#include <filesystem>

#include <rebirth/graphics/vulkan/descriptor_manager.h>
#include <rebirth/graphics/vulkan/resources.h>
#include <rebirth/graphics/vulkan/swapchain.h>

#include <tracy/TracyVulkan.hpp>

constexpr int FRAMES_IN_FLIGHT = 2;

namespace vulkan
{
    class Graphics
    {
    public:
        Graphics() = default;
        Graphics(Graphics const &) = delete;
        void operator=(Graphics const &) = delete;

        void initialize(SDL_Window *window);
        void destroy();

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
        DescriptorManager &getDescriptorManager() { return descriptorManager; }
        Image &getColorImage() { return colorImage; }
        Image &getDepthImage() { return depthImage; }
        VkPhysicalDeviceFeatures &getDeviceFeatures() { return deviceFeatures; }
        VkPhysicalDeviceProperties &getDevicePropertices() { return deviceProperties; }
        TracyVkCtx &getTracyContext() { return tracyVkCtx[currentFrame]; } // tracy profiler
        uint32_t getCurrentFrame() { return currentFrame; }
        
        // Features support
        bool supportTimestamps();

        // Resources
        void createImage(Image &image, ImageCreateInfo &createInfo, bool generateMipmaps);
        void createImageFromFile(Image &image, ImageCreateInfo &createInfo, std::filesystem::path path);
        void createImageFromMemory(Image &image, ImageCreateInfo &createInfo, unsigned char *data, int size);
        void createLoadImage(Image &image, ImageCreateInfo &createInfo, unsigned char *data, uint32_t size);
        void createCubemapImage(Image &image, ImageCreateInfo &createInfo, std::filesystem::path dir);
        void generateMipmaps(Image &image);
        void copyImage(VkCommandBuffer cmd, VkImage src, VkImage dst, uint32_t width, uint32_t height);
        void destroyImage(Image &image);

        void createBuffer(Buffer &buffer, BufferCreateInfo &createInfo);
        void destroyBuffer(Buffer &buffer);

        VkImageView createImageView(VkImage image, VkImageViewType viewType, VkFormat format, VkImageSubresourceRange subresourceRange);
        VkSampler createSampler(VkFilter minFilter, VkFilter magFilter, VkSamplerAddressMode samplerMode, float maxLod);

        // Query
        VkQueryPool createQueryPool(VkQueryType type, uint32_t queryCount);
        void resetQueryPool(VkCommandBuffer cmd, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
        void writeTimestamp(VkCommandBuffer cmd, VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query);

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
        VkPipelineLayout
        createPipelineLayout(VkDescriptorSetLayout *setLayout, VkPushConstantRange *pushConstant);

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

        void setupImGui();

        void createImages();

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

        DescriptorManager descriptorManager;

        VkCommandPool commandPool{VK_NULL_HANDLE};
        std::array<VkCommandBuffer, FRAMES_IN_FLIGHT> commandBuffers;

        // Sync primitives (per swapchain image)
        std::array<VkSemaphore, FRAMES_IN_FLIGHT> acquireSemaphores;
        std::array<VkFence, FRAMES_IN_FLIGHT> finishRenderFences;
        std::vector<VkSemaphore> submitSemaphores;

        vulkan::Image colorImage;
        vulkan::Image depthImage;

        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;

        std::array<TracyVkCtx, FRAMES_IN_FLIGHT> tracyVkCtx;

        uint32_t currentFrame = 0;
        bool resizeRequested = false;
    };

} // namespace vulkan

extern vulkan::Graphics g_graphics;
