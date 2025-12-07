#pragma once

// clang-format off
#include <volk.h>
#include <vk_mem_alloc.h>
// clang-format on

#include "core/application.h"
#include "core/stl.h"

#include "render/render_device.h"
#include "render/render_types.h"
#include "render/vulkan/vulkan_config.h"
#include "render/vulkan/vulkan_descriptor_set_writer.h"
#include "render/vulkan/vulkan_swapchain.h"
#include "render/vulkan/vulkan_window_system.h"

#ifdef ENABLE_VULKAN_PROFILE
#include <tracy/TracyVulkan.hpp>
#endif

class VulkanRenderDevice : public RenderDevice
{
public:
    VulkanRenderDevice(Application *application);
    virtual ~VulkanRenderDevice();

    virtual SharedPtr<Buffer>          createBuffer(const BufferCreateInfo &createInfo) override final;
    virtual SharedPtr<Image>           createImage(const ImageCreateInfo &createInfo) override final;
    virtual SharedPtr<Image>           createImageView(const ImageViewCreateInfo &createInfo) override final;
    virtual SharedPtr<Sampler>         createSampler(const SamplerCreateInfo &createInfo) override final;
    virtual SharedPtr<PipelineLayout>  createPipelineLayout(const PipelineLayoutCreateInfo &createInfo) override final;
    virtual SharedPtr<RenderPipeline>  createRenderPipeline(const RenderPipelineCreateInfo &createInfo) override final;
    virtual SharedPtr<ComputePipeline> createComputePipeline(const ComputePipelineCreateInfo &createInfo) override final;

    virtual void destroyBuffer(SharedPtr<Buffer> buffer) override final;
    virtual void destroyImage(SharedPtr<Image> image) override final;
    virtual void destroySampler(SharedPtr<Sampler> sampler) override final;
    virtual void destroyPipelineLayout(SharedPtr<PipelineLayout> layout) override final;
    virtual void destroyPipeline(SharedPtr<RenderPipeline> pipeline) override final;
    virtual void destroyPipeline(SharedPtr<ComputePipeline> pipeline) override final;

    virtual void uploadBufferData(SharedPtr<Buffer> buffer, void *data, size_t size) override final;

    virtual SharedPtr<CommandBuffer> beginCommandBuffer() override final;
    virtual void           endCommandBuffer(SharedPtr<CommandBuffer> commandBuffer) override final;
    virtual void           submitCommandBuffer(SharedPtr<CommandBuffer> commandBuffer) override final;
    virtual void           draw(SharedPtr<CommandBuffer> commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override final;
    virtual void           drawIndexed(SharedPtr<CommandBuffer> commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) override final;
    virtual void           bindPipeline(SharedPtr<CommandBuffer> commandBuffer, SharedPtr<RenderPipeline> pipeline) override final;
    virtual void           bindPipeline(SharedPtr<CommandBuffer> commandBuffer, SharedPtr<ComputePipeline> pipeline) override final;
    virtual void           bindVertexBuffer(SharedPtr<CommandBuffer> commandBuffer, SharedPtr<Buffer> vertexBuffer) override final;
    virtual void           beginRendering(SharedPtr<CommandBuffer> commandBuffer, const RenderingInfo &renderInfo) override final;
    virtual void           endRendering(SharedPtr<CommandBuffer> commandBuffer) override final;
    virtual void           setViewport(SharedPtr<CommandBuffer> commandBuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height) override final;
    virtual void           setScissor(SharedPtr<CommandBuffer> commandBuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height) override final;

    virtual void deviceWaitIdle() override final;

    virtual SharedPtr<Image> getSwapchainImage() override final;

private:
    void createInstance();
    void createDevice();
    void createAllocator();

    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool start);
    void            flushCommandBuffer(VkCommandBuffer cmd, VkQueue queue, VkCommandPool pool, bool free);

    VkInstance   instance = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    UniquePtr<VulkanWindowSystem> windowSystem;

#ifdef ENABLE_VULKAN_DEBUG
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
#endif

    uint32_t graphicsQueueIndex = UINT32_MAX;
    uint32_t computeQueueIndex = UINT32_MAX;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE;

    VkPhysicalDevice           physicalDevice = VK_NULL_HANDLE;
    VkDevice                   device = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures   deviceFeatures;

    VmaAllocator allocator = VK_NULL_HANDLE;

    VulkanSwapchain swapchain;

    VkCommandPool                            commandPool;
    Array<VkCommandBuffer, FRAMES_IN_FLIGHT> commandBuffers;
    Array<VkSemaphore, FRAMES_IN_FLIGHT>     acquireSemaphores;
    Array<VkFence, FRAMES_IN_FLIGHT>         finishRenderFences;
    Vector<VkSemaphore>                      submitSemaphores;

    VkDescriptorPool          descriptorPool;
    VkDescriptorSet           descriptorSet;
    VkDescriptorSetLayout     descriptorSetLayout;
    VulkanDescriptorSetWriter descriptorSetWriter;

#ifdef ENABLE_VULKAN_PROFILE
    Array<TracyVkCtx, FRAMES_IN_FLIGHT> tracyVkCtx;
#endif

    uint32_t maxSampleCount = VK_SAMPLE_COUNT_1_BIT;
    uint32_t currentFrame = 0;
    bool     resizeRequested = false;
};