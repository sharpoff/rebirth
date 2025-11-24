#pragma once

#include <volk.h>

#include <vk_mem_alloc.h>

#include "EASTL/array.h"

#include "core/application.h"
#include "render/graphics_types.h"
#include "render/render_device.h"
#include "render/vulkan/vulkan_swapchain.h"
#include "render/vulkan/vulkan_window_system.h"
#include "render/vulkan/vulkan_config.h"

#ifdef ENABLE_VULKAN_PROFILE
#include <tracy/TracyVulkan.hpp>
#endif

class VulkanRenderDevice : public RenderDevice
{
public:
    VulkanRenderDevice(Application *application);
    virtual ~VulkanRenderDevice();

    virtual Buffer  *createBuffer(const BufferCreateInfo &createInfo) override final;
    virtual Texture *createTexture(const TextureCreateInfo &createInfo) override final;
    virtual Texture *createTextureView(const TextureViewCreateInfo &createInfo) override final;
    virtual Sampler *createSampler(const SamplerCreateInfo &createInfo) override final;
    virtual PipelineLayout *createPipelineLayout(const PipelineLayoutCreateInfo &createInfo) override final;
    virtual RenderPipeline *createRenderPipeline(const RenderPipelineCreateInfo &createInfo) override final;
    virtual ComputePipeline *createComputePipeline(const ComputePipelineCreateInfo &createInfo) override final;

    virtual void destroyBuffer(Buffer *buffer) override final;
    virtual void destroyTexture(Texture *texture) override final;
    virtual void destroySampler(Sampler *sampler) override final;
    virtual void destroyPipelineLayout(PipelineLayout *layout) override final;
    virtual void destroyPipeline(RenderPipeline *pipeline) override final;
    virtual void destroyPipeline(ComputePipeline *pipeline) override final;

    virtual CommandBuffer *beginCommandBuffer() override final;
    virtual void submitCommandBuffer(CommandBuffer *commandBuffer) override final;
    virtual void draw(CommandBuffer *commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override final;
    virtual void drawIndexed(CommandBuffer *commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override final;
    virtual void bindPipeline(CommandBuffer *commandBuffer, RenderPipeline *pipeline) override final;
    virtual void bindPipeline(CommandBuffer *commandBuffer, ComputePipeline *pipeline) override final;

private:
    void createInstance();
    void createDevice();
    void createAllocator();

    VkInstance instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VulkanWindowSystem *windowSystem;

#ifdef ENABLE_VULKAN_DEBUG
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
#endif

    uint32_t graphicsQueueIndex = UINT32_MAX;
    uint32_t computeQueueIndex = UINT32_MAX;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    VmaAllocator allocator = VK_NULL_HANDLE;

    VulkanSwapchain swapchain;

    VkCommandPool commandPool;
    eastl::array<VkCommandBuffer, FRAMES_IN_FLIGHT> commandBuffers;
    eastl::array<VkSemaphore, FRAMES_IN_FLIGHT> acquireSemaphores;
    eastl::array<VkFence, FRAMES_IN_FLIGHT> finishRenderFences;
    eastl::vector<VkSemaphore> submitSemaphores;

#ifdef ENABLE_VULKAN_PROFILE
    eastl::array<TracyVkCtx, FRAMES_IN_FLIGHT> tracyVkCtx;
#endif

    uint32_t maxSampleCount = VK_SAMPLE_COUNT_1_BIT;
    uint32_t currentFrame = 0;
    bool resizeRequested = false;
};