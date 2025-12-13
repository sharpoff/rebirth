#include "render/vulkan/vulkan_render_device.h"

#include "core/stl.h"
#include "core/logger.h"

#include "render/render_types.h"
#include "render/vulkan/vulkan_config.h"
#include "render/vulkan/vulkan_helpers.h"
#include "render/vulkan/vulkan_sdl_window_system.h"
#include "render/vulkan/vulkan_swapchain.h"
#include "render/vulkan/vulkan_types.h"

#include "utility/common.h"
#include <cstddef>
#include <vulkan/vulkan_core.h>

// TODO: add anisotropy feature for sampler
// TODO: add stencil testing when creating render pipeline

#ifdef ENABLE_VULKAN_DEBUG
VkBool32 VKAPI_PTR vulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void                                       *pUserData)
{
    const char *type = "UNDEFINED";
    if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
        type = "GENERAL";
    else if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
        type = "VALIDATION";
    else if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
        type = "PERFORMANCE";

    const char *severity = "UNDEFINED";
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
        severity = "INFO";
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        severity = "ERROR";
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        severity = "VERBOSE";
    else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        severity = "WARNING";

    logger::log("[%s][VULKAN][%s / %s] %s\n", __TIME__, type, severity, pCallbackData->pMessage);

    return VK_FALSE;
}
#endif

VulkanRenderDevice::VulkanRenderDevice(Application *application)
{
#ifdef WINDOW_SYSTEM_SDL
    windowSystem = eastl::make_unique<VulkanSDLWindowSystem>(application->getHandle());
#endif

    createInstance();

    windowSystem->createSurface(instance, &surface);

    createDevice();

    VkSampleCountFlags supportedSampleCount = std::min(deviceProperties.limits.framebufferColorSampleCounts, deviceProperties.limits.framebufferDepthSampleCounts);
    if (supportedSampleCount & VK_SAMPLE_COUNT_64_BIT)
        maxSampleCount = VK_SAMPLE_COUNT_64_BIT;
    if (supportedSampleCount & VK_SAMPLE_COUNT_32_BIT)
        maxSampleCount = VK_SAMPLE_COUNT_32_BIT;
    if (supportedSampleCount & VK_SAMPLE_COUNT_16_BIT)
        maxSampleCount = VK_SAMPLE_COUNT_16_BIT;
    if (supportedSampleCount & VK_SAMPLE_COUNT_8_BIT)
        maxSampleCount = VK_SAMPLE_COUNT_8_BIT;
    if (supportedSampleCount & VK_SAMPLE_COUNT_4_BIT)
        maxSampleCount = VK_SAMPLE_COUNT_4_BIT;

    createAllocator();

    VulkanSwapchainCreateInfo swapchaincreateInfo;
    swapchaincreateInfo.device = device;
    swapchaincreateInfo.physicalDevice = physicalDevice;
    swapchaincreateInfo.pWindowSystem = windowSystem.get();
    swapchaincreateInfo.surface = surface;
    swapchain.create(swapchaincreateInfo);

    // create command pool
    VkCommandPoolCreateInfo commandPoolCI = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    commandPoolCI.queueFamilyIndex = graphicsQueueIndex;
    commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(device, &commandPoolCI, nullptr, &commandPool));

    // create command buffers
    VkCommandBufferAllocateInfo bufferAllocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    bufferAllocInfo.commandPool = commandPool;
    bufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    bufferAllocInfo.commandBufferCount = commandBuffers.size();
    VK_CHECK(vkAllocateCommandBuffers(device, &bufferAllocInfo, commandBuffers.data()));

    // create syncronizaiton objects
    submitSemaphores.resize(swapchain.getImagesCount());
    for (size_t i = 0; i < submitSemaphores.size(); i++) {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &submitSemaphores[i]));
    }

    for (unsigned int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VK_CHECK(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &acquireSemaphores[i]));

        VkFenceCreateInfo fenceCreateInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &finishRenderFences[i]));
    }

    // profiler context
#ifdef ENABLE_VULKAN_PROFILE
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        tracyVkCtx[i] = TracyVkContext(physicalDevice, device, graphicsQueue, commandBuffers[i]);
    }
#endif

    // descriptors
    Vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024},
    };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptorPoolCreateInfo.maxSets = 0;
    for (auto &poolSize : poolSizes) {
        descriptorPoolCreateInfo.maxSets += poolSize.descriptorCount;
    }
    descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
    VK_CHECK(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool));
}

VulkanRenderDevice::~VulkanRenderDevice()
{
    vkDeviceWaitIdle(device);

    swapchain.destroy(device);

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkDestroyCommandPool(device, commandPool, nullptr);

    for (VkSemaphore &semaphore : submitSemaphores) {
        vkDestroySemaphore(device, semaphore, nullptr);
    }

    for (unsigned int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, acquireSemaphores[i], nullptr);
        vkDestroyFence(device, finishRenderFences[i], nullptr);
    }

    vmaDestroyAllocator(allocator);

    vkDestroyDevice(device, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);

#ifdef ENABLE_VULKAN_DEBUG
    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif

    vkDestroyInstance(instance, nullptr);
}

SharedPtr<Buffer> VulkanRenderDevice::createBuffer(const BufferCreateInfo &createInfo)
{
    assert(createInfo.size > 0);

    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = createInfo.size;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = vulkan::getBufferUsageFlags(createInfo.usage);

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocInfo.priority = 1.0;

    SharedPtr<VulkanBuffer> buffer = eastl::make_shared<VulkanBuffer>();
    buffer->size = createInfo.size;
    buffer->usage = createInfo.usage;
    VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer->buffer, &buffer->allocation.handle, &buffer->allocation.info));

    if (bufferInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkBufferDeviceAddressInfo deviceAddressInfo = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR};
        deviceAddressInfo.buffer = buffer->buffer;
        buffer->address = vkGetBufferDeviceAddress(device, &deviceAddressInfo);
    }

    return buffer;
}

SharedPtr<Image> VulkanRenderDevice::createImage(const ImageCreateInfo &createInfo)
{
    assert(createInfo.width != 0 && createInfo.height != 0);

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = vulkan::getImageType(createInfo.type);
    imageInfo.format = vulkan::getFormat(createInfo.format);
    imageInfo.extent.width = createInfo.width;
    imageInfo.extent.height = createInfo.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = createInfo.mipLevels;
    imageInfo.arrayLayers = createInfo.arrayLayers;
    imageInfo.samples = vulkan::getSampleCount(createInfo.sampleCount);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = vulkan::getImageUsageFlags(createInfo.usage);
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    SharedPtr<VulkanImage> image = eastl::make_shared<VulkanImage>();
    image->width = createInfo.width;
    image->height = createInfo.height;
    image->layerCount = createInfo.arrayLayers;
    image->levelCount = createInfo.mipLevels;
    image->sampleCount = createInfo.sampleCount;
    image->type = createInfo.type;
    image->usage = createInfo.usage;
    image->format = createInfo.format;
    image->isSwapchain = false;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    VK_CHECK(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image->image, &image->allocation.handle, &image->allocation.info));
    assert(image->image != VK_NULL_HANDLE);

    return image;
}

SharedPtr<Image> VulkanRenderDevice::createImageView(const ImageViewCreateInfo &createInfo)
{
    VulkanImage *pViewedImage = (VulkanImage *)createInfo.image.get();
    assert(pViewedImage);

    SharedPtr<VulkanImage> imageView = eastl::make_shared<VulkanImage>();
    imageView->pViewedImage = pViewedImage;
    imageView->width = pViewedImage->width;
    imageView->height = pViewedImage->height;
    imageView->layerCount = pViewedImage->layerCount;
    imageView->levelCount = pViewedImage->levelCount;
    imageView->sampleCount = pViewedImage->sampleCount;
    imageView->type = pViewedImage->type;
    imageView->usage = pViewedImage->usage;
    imageView->format = pViewedImage->format;

    VkImageViewCreateInfo imageViewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    imageViewInfo.image = pViewedImage->image;
    imageViewInfo.viewType = vulkan::getImageViewType(pViewedImage->type);
    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.format = vulkan::getFormat(pViewedImage->format);
    imageViewInfo.subresourceRange = vulkan::getImageSubresourceRange(pViewedImage);

    VK_CHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &imageView->view));

    return imageView;
}

SharedPtr<Sampler> VulkanRenderDevice::createSampler(const SamplerCreateInfo &createInfo)
{
    VkSamplerCreateInfo samplerCreateInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerCreateInfo.minFilter = vulkan::getFilter(createInfo.minFilter);
    samplerCreateInfo.magFilter = vulkan::getFilter(createInfo.magFilter);
    samplerCreateInfo.addressModeU = vulkan::getSamplerAddressMode(createInfo.addressModeU);
    samplerCreateInfo.addressModeV = vulkan::getSamplerAddressMode(createInfo.addressModeV);
    samplerCreateInfo.addressModeW = vulkan::getSamplerAddressMode(createInfo.addressModeW);
    samplerCreateInfo.mipmapMode = vulkan::getSamplerMipmapMode(createInfo.mipmapMode);
    samplerCreateInfo.compareOp = vulkan::getCompareOp(createInfo.compareOp);
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerCreateInfo.maxLod = createInfo.maxLod;

    SharedPtr<VulkanSampler> sampler = eastl::make_shared<VulkanSampler>();
    assert(sampler);
    sampler->mipLodBias = createInfo.mipLodBias;
    sampler->minLod = createInfo.minLod;
    sampler->maxLod = createInfo.maxLod;
    sampler->maxAnisotropy = createInfo.maxAnisotropy;
    sampler->magFilter = createInfo.magFilter;
    sampler->minFilter = createInfo.minFilter;
    sampler->mipmapMode = createInfo.mipmapMode;
    sampler->addressModeU = createInfo.addressModeU;
    sampler->addressModeV = createInfo.addressModeV;
    sampler->addressModeW = createInfo.addressModeW;
    sampler->compareOp = createInfo.compareOp;
    VK_CHECK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &sampler->sampler));

    return sampler;
}

SharedPtr<PipelineLayout> VulkanRenderDevice::createPipelineLayout(const PipelineLayoutCreateInfo &createInfo)
{
    Vector<VkDescriptorSetLayout> descriptorSetLayouts;
    Vector<VkDescriptorSet> descriptorSets;

    if (!createInfo.descriptorSetLayouts.empty()) {
        descriptorSetLayouts.resize(createInfo.descriptorSetLayouts.size());

        for (size_t i = 0; i < createInfo.descriptorSetLayouts.size(); i++) {
            const Vector<DescriptorSetLayoutBinding> &bindings = createInfo.descriptorSetLayouts[i].bindings;

            Vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings(bindings.size());

            for (size_t j = 0; j < bindings.size(); j++) {
                descriptorSetLayoutBindings[j].binding = bindings[j].binding;
                descriptorSetLayoutBindings[j].descriptorType = vulkan::getDescriptorType(bindings[j].descriptorType);
                descriptorSetLayoutBindings[j].descriptorCount = bindings[j].descriptorCount;
                descriptorSetLayoutBindings[j].stageFlags = vulkan::getShaderStageFlags(bindings[j].stageMask);
            }

            VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
            layoutCreateInfo.bindingCount = descriptorSetLayoutBindings.size();
            layoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

            VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, nullptr, &descriptorSetLayouts[i]));
        }

        descriptorSets.resize(descriptorSetLayouts.size());

        // allocate descriptor sets
        VkDescriptorSetAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = descriptorSets.size();
        allocInfo.pSetLayouts = descriptorSetLayouts.data();
        VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()));
    }

    VkPipelineLayoutCreateInfo layoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    layoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
    layoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();

    SharedPtr<VulkanPipelineLayout> pipelineLayout = eastl::make_shared<VulkanPipelineLayout>();
    pipelineLayout->descriptorSetLayouts = descriptorSetLayouts;
    pipelineLayout->descriptorSets = descriptorSets;
    VK_CHECK(vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, &pipelineLayout->layout));

    return pipelineLayout;
}

SharedPtr<RenderPipeline> VulkanRenderDevice::createRenderPipeline(const RenderPipelineCreateInfo &createInfo)
{
    VulkanPipelineLayout *vulkanPipelineLayout = (VulkanPipelineLayout *)createInfo.pipelineLayout.get();
    assert(vulkanPipelineLayout);

    Vector<VkPipelineShaderStageCreateInfo> stages;

    VkShaderModule vertexModule = VK_NULL_HANDLE;
    VkShaderModule fragmentModule = VK_NULL_HANDLE;
    VkShaderModule tessellationControlModule = VK_NULL_HANDLE;
    VkShaderModule tessellationEvaluationModule = VK_NULL_HANDLE;

    if (!createInfo.vertexCode.empty()) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleCreateInfo.codeSize = createInfo.vertexCode.size();
        shaderModuleCreateInfo.pCode = (uint32_t*)createInfo.vertexCode.data();
        VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &vertexModule));

        VkPipelineShaderStageCreateInfo &stage = stages.emplace_back();
        stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage.module = vertexModule;
        stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage.pName = "main";
    }

    if (!createInfo.fragmentCode.empty()) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleCreateInfo.codeSize = createInfo.fragmentCode.size();
        shaderModuleCreateInfo.pCode = (uint32_t*)createInfo.fragmentCode.data();
        VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &fragmentModule));

        VkPipelineShaderStageCreateInfo &stage = stages.emplace_back();
        stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage.module = fragmentModule;
        stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage.pName = "main";
    }

    if (!createInfo.tessellationControlCode.empty()) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleCreateInfo.codeSize = createInfo.tessellationControlCode.size();
        shaderModuleCreateInfo.pCode = (uint32_t*)createInfo.tessellationControlCode.data();
        VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &tessellationControlModule));

        VkPipelineShaderStageCreateInfo &stage = stages.emplace_back();
        stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage.module = tessellationControlModule;
        stage.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        stage.pName = "main";
    }

    if (!createInfo.tessellationEvaluationCode.empty()) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleCreateInfo.codeSize = createInfo.tessellationEvaluationCode.size();
        shaderModuleCreateInfo.pCode = (uint32_t*)createInfo.tessellationEvaluationCode.data();
        VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &tessellationEvaluationModule));

        VkPipelineShaderStageCreateInfo &stage = stages.emplace_back();
        stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage.module = tessellationEvaluationModule;
        stage.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        stage.pName = "main";
    }

    Vector<VkVertexInputBindingDescription> vertexBindingDescriptions(createInfo.vertexBindings.size());
    for (size_t i = 0; i < createInfo.vertexBindings.size(); i++) {
        const VertexBinding &binding = createInfo.vertexBindings[i];
        vertexBindingDescriptions[i].binding = binding.binding;
        vertexBindingDescriptions[i].stride = binding.stride;
        vertexBindingDescriptions[i].inputRate = binding.inputRate == VERTEX_INPUT_RATE_VERTEX ? VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
    }

    Vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions(createInfo.vertexAttributes.size());
    for (size_t i = 0; i < createInfo.vertexAttributes.size(); i++) {
        const VertexAttribute &attrib = createInfo.vertexAttributes[i];
        vertexAttributeDescriptions[i].location = attrib.location;
        vertexAttributeDescriptions[i].binding = attrib.binding;
        vertexAttributeDescriptions[i].format = vulkan::getFormat(attrib.format);
        vertexAttributeDescriptions[i].offset = attrib.offset;
    }

    VkPipelineVertexInputStateCreateInfo vertexInputState = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputState.vertexAttributeDescriptionCount = vertexAttributeDescriptions.size();
    vertexInputState.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();
    vertexInputState.vertexBindingDescriptionCount = vertexBindingDescriptions.size();
    vertexInputState.pVertexBindingDescriptions = vertexBindingDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssemblyState.topology = vulkan::getPrimitiveTopology(createInfo.topology);

    VkPipelineTessellationStateCreateInfo tessellationState = {VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
    tessellationState.patchControlPoints = createInfo.patchControlPoints;

    vec2 windowSize = windowSystem->getWindowSize();
    uint32_t width = (uint32_t)windowSize.x;
    uint32_t height = (uint32_t)windowSize.y;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = width;
    viewport.height = height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = width;
    scissor.extent.height = height;

    VkPipelineViewportStateCreateInfo viewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizationState = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.polygonMode = vulkan::getPolygonMode(createInfo.polygonMode);
    rasterizationState.cullMode = vulkan::getCullMode(createInfo.cullMode);
    rasterizationState.frontFace = vulkan::getFrontFace(createInfo.frontFace);
    rasterizationState.depthBiasEnable = VK_FALSE;
    rasterizationState.depthBiasConstantFactor = 0.0f;
    rasterizationState.depthBiasClamp = 0.0f;
    rasterizationState.depthBiasSlopeFactor = 0.0f;
    rasterizationState.lineWidth = 1.0;

    VkPipelineMultisampleStateCreateInfo multisampleState = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampleState.rasterizationSamples = vulkan::getSampleCount(createInfo.sampleCount);
    if (deviceFeatures.sampleRateShading) {
        multisampleState.sampleShadingEnable = VK_TRUE;
        multisampleState.minSampleShading = 0.2f;
    } else {
        multisampleState.sampleShadingEnable = VK_FALSE;
    }

    const VkBool32 depthTestEnabled = createInfo.depthCompareOp != COMPARE_OP_ALWAYS;
    const VkBool32 depthWriteEnabled = createInfo.depthWriteEnable;

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencilState.depthTestEnable = depthTestEnabled || depthWriteEnabled;
    depthStencilState.depthWriteEnable = depthWriteEnabled;
    depthStencilState.depthCompareOp = vulkan::getCompareOp(createInfo.depthCompareOp);
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.stencilTestEnable = VK_FALSE; // TODO: stencil
    depthStencilState.front = {}; // TODO: stencil
    depthStencilState.back = {}; // TODO: stencil
    depthStencilState.minDepthBounds = 0.0f;
    depthStencilState.maxDepthBounds = 1.0f;

    const VkBool32 blendEnabled = createInfo.colorBlendOp != BLEND_OP_NONE;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
    colorBlendAttachmentState.blendEnable = blendEnabled;
    colorBlendAttachmentState.srcColorBlendFactor = vulkan::getBlendFactor(createInfo.colorBlendFactorSrc);
    colorBlendAttachmentState.dstColorBlendFactor = vulkan::getBlendFactor(createInfo.colorBlendFactorDst);
    colorBlendAttachmentState.colorBlendOp = vulkan::getBlendOp(createInfo.colorBlendOp);
    colorBlendAttachmentState.srcAlphaBlendFactor = vulkan::getBlendFactor(createInfo.alphaBlendFactorSrc);
    colorBlendAttachmentState.dstAlphaBlendFactor = vulkan::getBlendFactor(createInfo.alphaBlendFactorDst);
    colorBlendAttachmentState.alphaBlendOp = vulkan::getBlendOp(createInfo.alphaBlendOp);
    colorBlendAttachmentState.colorWriteMask = vulkan::getColorComponentFlags(createInfo.colorWriteMask);

    Vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(createInfo.colorAttachmentFormats.size(), colorBlendAttachmentState);

    VkPipelineColorBlendStateCreateInfo colorBlendState = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.attachmentCount = colorBlendAttachments.size();
    colorBlendState.pAttachments = colorBlendAttachments.data();

    Vector<VkDynamicState> dynamicStates;
    if ((createInfo.dynamicState & DYNAMIC_STATE_VIEWPORT) == DYNAMIC_STATE_VIEWPORT) {
        dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    }

    if ((createInfo.dynamicState & DYNAMIC_STATE_SCISSOR) == DYNAMIC_STATE_SCISSOR) {
        dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
    }

    VkPipelineDynamicStateCreateInfo dynamicState = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();

    Vector<VkFormat> colorAttachmentFormats(createInfo.colorAttachmentFormats.size());
    for (size_t i = 0; i < colorAttachmentFormats.size(); i++) {
        colorAttachmentFormats[i] = (vulkan::getFormat(createInfo.colorAttachmentFormats[i]));
    }

    VkPipelineRenderingCreateInfoKHR renderingInfo = {VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR};
    renderingInfo.colorAttachmentCount = colorAttachmentFormats.size();
    renderingInfo.pColorAttachmentFormats = colorAttachmentFormats.data();
    if (depthTestEnabled || depthWriteEnabled)
        renderingInfo.depthAttachmentFormat = vulkan::getFormat(createInfo.depthAttachmentFormat);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipelineCreateInfo.pNext = &renderingInfo;
    pipelineCreateInfo.stageCount = stages.size();
    pipelineCreateInfo.pStages = stages.data();
    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pTessellationState = &tessellationState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.layout = vulkanPipelineLayout->layout;

    SharedPtr<VulkanRenderPipeline> renderPipeline = eastl::make_shared<VulkanRenderPipeline>();
    renderPipeline->layout = createInfo.pipelineLayout;
    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &renderPipeline->pipeline));

    if (vertexModule)
        vkDestroyShaderModule(device, vertexModule, nullptr);
    if (fragmentModule)
        vkDestroyShaderModule(device, fragmentModule, nullptr);
    if (tessellationControlModule)
        vkDestroyShaderModule(device, tessellationControlModule, nullptr);
    if (tessellationEvaluationModule)
        vkDestroyShaderModule(device, tessellationEvaluationModule, nullptr);

    return renderPipeline;
}

SharedPtr<ComputePipeline> VulkanRenderDevice::createComputePipeline(const ComputePipelineCreateInfo &createInfo)
{
    VulkanPipelineLayout *vulkanPipelineLayout = (VulkanPipelineLayout *)createInfo.pipelineLayout.get();
    assert(vulkanPipelineLayout);

    VkPipelineShaderStageCreateInfo computeStage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    VkShaderModule computeModule = VK_NULL_HANDLE;

    if (!createInfo.computeCode.empty()) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleCreateInfo.codeSize = createInfo.computeCode.size();
        shaderModuleCreateInfo.pCode = (uint32_t*)createInfo.computeCode.data();
        VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &computeModule));

        computeStage.module = computeModule;
        computeStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        computeStage.pName = "main";
    }

    assert(computeModule != VK_NULL_HANDLE);

    VkComputePipelineCreateInfo pipelineCreateInfo = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    pipelineCreateInfo.stage = computeStage;
    pipelineCreateInfo.layout = vulkanPipelineLayout->layout;

    SharedPtr<VulkanComputePipeline> computePipeline = eastl::make_shared<VulkanComputePipeline>();
    computePipeline->layout = createInfo.pipelineLayout;
    VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &computePipeline->pipeline));

    return computePipeline;
}

void VulkanRenderDevice::destroyBuffer(SharedPtr<Buffer> buffer)
{
    if (buffer) {
        VulkanBuffer *vulkanBuffer = static_cast<VulkanBuffer *>(buffer.get());

        if (vulkanBuffer->buffer != VK_NULL_HANDLE)
            vmaDestroyBuffer(allocator, vulkanBuffer->buffer, vulkanBuffer->allocation.handle);
    }
}

void VulkanRenderDevice::destroyImage(SharedPtr<Image> image)
{
    VulkanImage *vulkanImage = static_cast<VulkanImage *>(image.get());

    if (vulkanImage->view != VK_NULL_HANDLE)
        vkDestroyImageView(device, vulkanImage->view, nullptr);

    if (vulkanImage->image != VK_NULL_HANDLE)
        vmaDestroyImage(allocator, vulkanImage->image, vulkanImage->allocation.handle);
}

void VulkanRenderDevice::destroySampler(SharedPtr<Sampler> sampler)
{
    VulkanSampler *vulkanSampler = static_cast<VulkanSampler *>(sampler.get());

    if (vulkanSampler->sampler != VK_NULL_HANDLE)
        vkDestroySampler(device, vulkanSampler->sampler, nullptr);
}

void VulkanRenderDevice::destroyPipelineLayout(SharedPtr<PipelineLayout> layout)
{
    VulkanPipelineLayout *vulkanPipelineLayout = static_cast<VulkanPipelineLayout *>(layout.get());

    for (auto &descriptorSetLayout : vulkanPipelineLayout->descriptorSetLayouts)
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    if (vulkanPipelineLayout->layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(device, vulkanPipelineLayout->layout, nullptr);
}

void VulkanRenderDevice::destroyPipeline(SharedPtr<RenderPipeline> pipeline)
{
    VulkanRenderPipeline *vulkanRenderPipeline = static_cast<VulkanRenderPipeline *>(pipeline.get());

    if (vulkanRenderPipeline->pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(device, vulkanRenderPipeline->pipeline, nullptr);
}

void VulkanRenderDevice::destroyPipeline(SharedPtr<ComputePipeline> pipeline)
{
    VulkanComputePipeline *vulkanComputePipeline = static_cast<VulkanComputePipeline *>(pipeline.get());

    if (vulkanComputePipeline->pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(device, vulkanComputePipeline->pipeline, nullptr);
}

void VulkanRenderDevice::uploadBufferData(SharedPtr<Buffer> buffer, void *data, size_t size)
{
    assert(data && size > 0);
    VulkanBuffer *vulkanBuffer = static_cast<VulkanBuffer*>(buffer.get());

    const BufferCreateInfo createInfo = {
        .size = size,
        .usage = BUFFER_USAGE_TRANSFER_SRC,
    };

    SharedPtr<Buffer> staging = createBuffer(createInfo);
    VulkanBuffer *vkStaging = static_cast<VulkanBuffer*>(staging.get());
    memcpy(vkStaging->allocation.info.pMappedData, data, size);

    VK_CHECK(vmaFlushAllocation(allocator, vkStaging->allocation.handle, 0, VK_WHOLE_SIZE));

    VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkBufferCopy copyRegion = {0, 0, size};
    vkCmdCopyBuffer(copyCmd, vkStaging->buffer, vulkanBuffer->buffer, 1, &copyRegion);

    flushCommandBuffer(copyCmd, graphicsQueue, commandPool, true);

    destroyBuffer(staging);
}

void VulkanRenderDevice::uploadImageData(SharedPtr<Image> image, void *data, size_t size)
{
    assert(data && size > 0);
    VulkanImage *vulkanImage = static_cast<VulkanImage*>(image.get());

    const BufferCreateInfo createInfo = {
        .size = size,
        .usage = BUFFER_USAGE_TRANSFER_SRC,
    };

    SharedPtr<Buffer> staging = createBuffer(createInfo);
    VulkanBuffer *vkStaging = static_cast<VulkanBuffer*>(staging.get());
    memcpy(vkStaging->allocation.info.pMappedData, data, size);

    VK_CHECK(vmaFlushAllocation(allocator, vkStaging->allocation.handle, 0, VK_WHOLE_SIZE));

    VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    // transition image to transfer
    VkImageMemoryBarrier transferBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    transferBarrier.srcAccessMask = 0;
    transferBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    transferBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    transferBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    transferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    transferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    transferBarrier.image = vulkanImage->image;
    transferBarrier.subresourceRange = vulkan::getImageSubresourceRange(image.get());

    vkCmdPipelineBarrier(copyCmd,
        VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, // stages
        0,
        0, nullptr, // memory barriers
        0, nullptr, // buffer memory barriers
        1, &transferBarrier // image memory barriers
    );

    // copy
    VkBufferImageCopy copyRegion = {};
    copyRegion.imageSubresource = vulkan::getImageSubresourceLayers(image.get());
    copyRegion.imageExtent = {image->width, image->height, 1};

    vkCmdCopyBufferToImage(copyCmd, vkStaging->buffer, vulkanImage->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    // transition image to fragment shader
    VkImageMemoryBarrier fragmentBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    fragmentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    fragmentBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    fragmentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    fragmentBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    fragmentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    fragmentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    fragmentBarrier.image = vulkanImage->image;
    fragmentBarrier.subresourceRange = vulkan::getImageSubresourceRange(image.get());

    vkCmdPipelineBarrier(copyCmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // stages
        0,
        0, nullptr, // memory barriers
        0, nullptr, // buffer memory barriers
        1, &fragmentBarrier // image memory barriers
    );

    flushCommandBuffer(copyCmd, graphicsQueue, commandPool, true);

    destroyBuffer(staging);
}

SharedPtr<CommandBuffer> VulkanRenderDevice::beginCommandBuffer()
{
    SharedPtr<VulkanCommandBuffer> commandBuffer = eastl::make_shared<VulkanCommandBuffer>();

    VK_CHECK(vkWaitForFences(device, 1, &finishRenderFences[currentFrame], VK_TRUE, ~0ull));
    VK_CHECK(vkResetFences(device, 1, &finishRenderFences[currentFrame]));

    VkResult result = swapchain.acquireNextImage(device, acquireSemaphores[currentFrame]);
    if (resizeRequested || result == VK_ERROR_OUT_OF_DATE_KHR) {
        // recreateSwapchain(); // TODO:
        return VK_NULL_HANDLE;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LOGE("%s", "Failed to acquire swapchain image.");
        exit(EXIT_FAILURE);
    }

    commandBuffer->cmd = commandBuffers[currentFrame];
    VK_CHECK(vkResetCommandBuffer(commandBuffer->cmd, 0));

    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(commandBuffer->cmd, &beginInfo));

    return commandBuffer;
}

void VulkanRenderDevice::endCommandBuffer(SharedPtr<CommandBuffer> commandBuffer)
{
    assert(commandBuffer);
    VulkanCommandBuffer *vulkanCommandBuffer = static_cast<VulkanCommandBuffer*>(commandBuffer.get());
    VK_CHECK(vkEndCommandBuffer(vulkanCommandBuffer->cmd));
}

void VulkanRenderDevice::submitCommandBuffer(SharedPtr<CommandBuffer> commandBuffer)
{
    assert(commandBuffer);
    uint32_t imageIndex = swapchain.getImageIndex();

    VulkanCommandBuffer *vulkanCommandBuffer = static_cast<VulkanCommandBuffer*>(commandBuffer.get());

    // Submit
    VkSubmitInfo submit = {};
    VkPipelineStageFlags stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &acquireSemaphores[currentFrame];
    submit.pWaitDstStageMask = stages;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &vulkanCommandBuffer->cmd;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &submitSemaphores[imageIndex];
    VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submit, finishRenderFences[currentFrame]));

    // Present
    VkResult result = swapchain.present(graphicsQueue, submitSemaphores[imageIndex]);
    if (resizeRequested || result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // recreateSwapchain(); // TODO:
    }

    currentFrame = (currentFrame + 1) % FRAMES_IN_FLIGHT;
}

void VulkanRenderDevice::draw(SharedPtr<CommandBuffer> commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    assert(commandBuffer);
    VkCommandBuffer cmd = static_cast<VulkanCommandBuffer*>(commandBuffer.get())->cmd;
    vkCmdDraw(cmd, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanRenderDevice::drawIndexed(SharedPtr<CommandBuffer> commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    assert(commandBuffer);
    VkCommandBuffer cmd = static_cast<VulkanCommandBuffer*>(commandBuffer.get())->cmd;
    vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanRenderDevice::bindPipeline(SharedPtr<CommandBuffer> commandBuffer, SharedPtr<RenderPipeline> pipeline)
{
    assert(commandBuffer && pipeline);
    VkCommandBuffer cmd = static_cast<VulkanCommandBuffer*>(commandBuffer.get())->cmd;
    VulkanRenderPipeline *vulkanRenderPipeline = static_cast<VulkanRenderPipeline*>(pipeline.get());
    VulkanPipelineLayout *vulkanPipelineLayout = static_cast<VulkanPipelineLayout*>(pipeline->layout.get());

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPipeline->pipeline);
    if (!vulkanPipelineLayout->descriptorSets.empty())
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanPipelineLayout->layout, 0, vulkanPipelineLayout->descriptorSets.size(), vulkanPipelineLayout->descriptorSets.data(), 0, nullptr);
}

void VulkanRenderDevice::bindPipeline(SharedPtr<CommandBuffer> commandBuffer, SharedPtr<ComputePipeline> pipeline)
{
    assert(commandBuffer && pipeline);
    VkCommandBuffer cmd = static_cast<VulkanCommandBuffer*>(commandBuffer.get())->cmd;
    VulkanComputePipeline *vulkanComputePipeline = static_cast<VulkanComputePipeline*>(pipeline.get());
    VulkanPipelineLayout *vulkanPipelineLayout = static_cast<VulkanPipelineLayout*>(pipeline->layout.get());

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, vulkanComputePipeline->pipeline);
    if (!vulkanPipelineLayout->descriptorSets.empty())
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, vulkanPipelineLayout->layout, 0, vulkanPipelineLayout->descriptorSets.size(), vulkanPipelineLayout->descriptorSets.data(), 0, nullptr);
}

void VulkanRenderDevice::bindVertexBuffer(SharedPtr<CommandBuffer> commandBuffer, SharedPtr<Buffer> vertexBuffer)
{
    assert(commandBuffer && vertexBuffer);
    VkCommandBuffer cmd = static_cast<VulkanCommandBuffer*>(commandBuffer.get())->cmd;
    VulkanBuffer *buffer = static_cast<VulkanBuffer*>(vertexBuffer.get());

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &buffer->buffer, &offset);
}

void VulkanRenderDevice::beginRendering(SharedPtr<CommandBuffer> commandBuffer, const RenderingInfo &renderInfo)
{
    VkCommandBuffer cmd = static_cast<VulkanCommandBuffer *>(commandBuffer.get())->cmd;

    vec2 windowSize = windowSystem->getWindowSize();
    uint32_t width = (uint32_t)windowSize.x;
    uint32_t height = (uint32_t)windowSize.y;

    VkRenderingAttachmentInfo depthInfo = {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    if (renderInfo.depthAttachment) {
        VulkanImage *depthImage = static_cast<VulkanImage *>(renderInfo.depthAttachment->image.get());

        depthInfo.clearValue.depthStencil = {0.0f, 0};
        depthInfo.loadOp = renderInfo.depthAttachment->load ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthInfo.storeOp = renderInfo.depthAttachment->store ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthInfo.imageView = depthImage->view;
        depthInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    }

    Vector<VkImageMemoryBarrier> imageMemoryBarriers;
    VkPipelineStageFlags srcPipelineStage = 0;
    VkPipelineStageFlags dstPipelineStage = 0;

    Vector<VkRenderingAttachmentInfo> colorAttachments(renderInfo.colorAttachments.size());
    for (size_t i = 0; i < renderInfo.colorAttachments.size(); i++) {
        const AttachmentResource &attachment = renderInfo.colorAttachments[i];
        VulkanImage *image = static_cast<VulkanImage *>(attachment.image.get());

        VkRenderingAttachmentInfo &info = colorAttachments[i];
        info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        info.clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        info.imageView = image->view;
        info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        info.loadOp = attachment.load ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR;
        info.storeOp = attachment.store ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;

        VkImageMemoryBarrier &barrier = imageMemoryBarriers.emplace_back();
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image->image;
        barrier.subresourceRange = vulkan::getImageSubresourceRange(image);
        srcPipelineStage |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dstPipelineStage |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }

    if (renderInfo.depthAttachment) {
        VulkanImage *depthImage = static_cast<VulkanImage *>(renderInfo.depthAttachment->image.get());

        VkImageMemoryBarrier &depthBarrier = imageMemoryBarriers.emplace_back();
        depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        depthBarrier.srcAccessMask = 0;
        depthBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        depthBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthBarrier.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        depthBarrier.image = depthImage->image;
        depthBarrier.subresourceRange = vulkan::getImageSubresourceRange(depthImage);
        srcPipelineStage |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dstPipelineStage |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    }

    // insert barriers
    vkCmdPipelineBarrier(cmd,
        srcPipelineStage, dstPipelineStage, // stages
        0,
        0, nullptr, // memory barriers
        0, nullptr, // buffer memory barriers
        imageMemoryBarriers.size(), imageMemoryBarriers.data() // image memory barriers
    );

    // begin rendering
    VkRenderingInfo vulkanRenderingInfo = {VK_STRUCTURE_TYPE_RENDERING_INFO};
    vulkanRenderingInfo.renderArea = {};
    vulkanRenderingInfo.renderArea.extent = {width, height};
    vulkanRenderingInfo.pDepthAttachment = &depthInfo;
    vulkanRenderingInfo.pColorAttachments = colorAttachments.data();
    vulkanRenderingInfo.colorAttachmentCount = colorAttachments.size();
    vulkanRenderingInfo.layerCount = 1;

    vkCmdBeginRendering(cmd, &vulkanRenderingInfo);

    setViewport(commandBuffer, 0.0f, 0.0f, width, height);
    setScissor(commandBuffer, 0.0f, 0.0f, width, height);

    commandBuffer->renderInfos.push(renderInfo);
}

void VulkanRenderDevice::endRendering(SharedPtr<CommandBuffer> commandBuffer)
{
    VkCommandBuffer cmd = static_cast<VulkanCommandBuffer *>(commandBuffer.get())->cmd;
    vkCmdEndRendering(cmd);

    auto &colorAttachments = commandBuffer->renderInfos.front().colorAttachments;
    for (auto &attachment : colorAttachments) {
        if (!attachment.image->isSwapchain) { // find swapchain image
            continue;
        }

        VulkanImage *swapchainImage = static_cast<VulkanImage*>(attachment.image.get());

        VkImageMemoryBarrier swapchainBarrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        swapchainBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        swapchainBarrier.dstAccessMask = 0;
        swapchainBarrier.oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        swapchainBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        swapchainBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        swapchainBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        swapchainBarrier.image = swapchainImage->image;
        swapchainBarrier.subresourceRange.levelCount = 1;
        swapchainBarrier.subresourceRange.baseMipLevel = 0;
        swapchainBarrier.subresourceRange.baseArrayLayer = 0;
        swapchainBarrier.subresourceRange.layerCount = 1;
        swapchainBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

        vkCmdPipelineBarrier(cmd,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_NONE, // stages
            0,
            0, nullptr, // memory barriers
            0, nullptr, // buffer memory barriers
            1, &swapchainBarrier // image memory barriers
        );

        break;
    }

    commandBuffer->renderInfos.pop();
}

void VulkanRenderDevice::setViewport(SharedPtr<CommandBuffer> commandBuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    VkCommandBuffer cmd = static_cast<VulkanCommandBuffer *>(commandBuffer.get())->cmd;

    VkViewport viewport = {};
    viewport.x = x;
    viewport.y = y;
    viewport.width = width;
    viewport.height = height;
    vkCmdSetViewport(cmd, 0, 1, &viewport);
}

void VulkanRenderDevice::setScissor(SharedPtr<CommandBuffer> commandBuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    VkCommandBuffer cmd = static_cast<VulkanCommandBuffer *>(commandBuffer.get())->cmd;

    VkRect2D scissor = {};
    scissor.offset.x = x;
    scissor.offset.y = y;
    scissor.extent.width = width;
    scissor.extent.height = height;
    vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void VulkanRenderDevice::writeDescriptor(uint32_t binding, SharedPtr<Buffer> buffer, DescriptorType type, uint32_t dstArrayElement)
{
    VulkanBuffer *vulkanBuffer = static_cast<VulkanBuffer *>(buffer.get());
    descriptorSetWriter.write(binding, vulkanBuffer->buffer, vulkanBuffer->size, vulkan::getDescriptorType(type), dstArrayElement);
}

void VulkanRenderDevice::writeDescriptor(uint32_t binding, SharedPtr<Image> imageView, SharedPtr<Sampler> sampler, DescriptorType type, uint32_t dstArrayElement)
{
    VulkanImage *vulkanImageView = static_cast<VulkanImage *>(imageView.get());
    VulkanSampler *vulkanSampler = static_cast<VulkanSampler *>(sampler.get());
    descriptorSetWriter.write(binding, vulkanImageView->view, vulkanSampler->sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, vulkan::getDescriptorType(type), dstArrayElement);
}

void VulkanRenderDevice::updateDescriptor(SharedPtr<PipelineLayout> layout, uint32_t set)
{
    VulkanPipelineLayout *vulkanPipelineLayout = static_cast<VulkanPipelineLayout*>(layout.get());
    assert(set >= 0 && set < vulkanPipelineLayout->descriptorSets.size()); // bounds check
    descriptorSetWriter.update(device, vulkanPipelineLayout->descriptorSets[set]);
    descriptorSetWriter.clear();
}

void VulkanRenderDevice::deviceWaitIdle()
{
    vkDeviceWaitIdle(device);
}

SharedPtr<Image> VulkanRenderDevice::getSwapchainImage()
{
    return swapchain.getImage();
}

void VulkanRenderDevice::createInstance()
{
    VK_CHECK(volkInitialize());

    VkApplicationInfo appCI = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appCI.apiVersion = VULKAN_API_VERSION;
    appCI.pEngineName = "Engine";
    appCI.pApplicationName = "Application";
    appCI.engineVersion = 0;
    appCI.applicationVersion = 0;

    Vector<const char *> instanceExtensions = windowSystem->getInstanceExtensions();

#ifdef ENABLE_VULKAN_DEBUG
    instanceExtensions.push_back("VK_EXT_debug_utils");
#endif

    Vector<const char *> instanceLayers;
#ifdef ENABLE_VULKAN_DEBUG
    instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    VkInstanceCreateInfo instanceInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    instanceInfo.pApplicationInfo = &appCI;
    instanceInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
    instanceInfo.ppEnabledLayerNames = instanceLayers.data();
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();

#ifdef ENABLE_VULKAN_DEBUG
    VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = vulkanDebugCallback,
        .pUserData = nullptr,
    };
    instanceInfo.pNext = &messengerInfo;

    VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &instance));
#else
    VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &instance));
#endif

    volkLoadInstance(instance);

#ifdef ENABLE_VULKAN_DEBUG
    VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &messengerInfo, nullptr, &debugMessenger));
#endif
}

void VulkanRenderDevice::createDevice()
{
    uint32_t physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
    assert(physicalDeviceCount > 0);

    Vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));

    // FIXME: find appropriate device
    physicalDevice = physicalDevices[0];
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    // find queue indices
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    Vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    VkBool32 presentSupport = VK_FALSE;

    uint32_t i = 0;
    for (VkQueueFamilyProperties queueFamily : queueFamilies) {
        if (queueFamily.queueCount <= 0)
            continue;

        if (graphicsQueueIndex == UINT32_MAX && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport));
            if (presentSupport)
                graphicsQueueIndex = i;
        }

        if (computeQueueIndex == UINT32_MAX && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
            computeQueueIndex = i;
        i++;
    }

    assert(graphicsQueueIndex != UINT32_MAX && computeQueueIndex != UINT32_MAX);
    Set<uint32_t> uniqueQueueIndices = {graphicsQueueIndex, computeQueueIndex};

    Vector<VkDeviceQueueCreateInfo> deviceQueueCI;

    float queuePriority = 1.0f;
    for (auto &index : uniqueQueueIndices) {
        VkDeviceQueueCreateInfo queueCI = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queueCI.queueCount = 1;
        queueCI.pQueuePriorities = &queuePriority;
        queueCI.queueFamilyIndex = index;
        deviceQueueCI.push_back(queueCI);
    }

    // VK 1.0 features
    deviceFeatures = {};
    deviceFeatures.fillModeNonSolid = VK_TRUE;
    deviceFeatures.shaderSampledImageArrayDynamicIndexing = VK_TRUE;

    // VK 1.2 features
    VkPhysicalDeviceVulkan12Features features12 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    features12.bufferDeviceAddress = VK_TRUE;

    // Dynamic rendering features
    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES};
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
    dynamicRenderingFeatures.pNext = &features12;

    // sync2
    VkPhysicalDeviceSynchronization2Features sync2Features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES};
    sync2Features.synchronization2 = VK_TRUE;
    sync2Features.pNext = &dynamicRenderingFeatures;

    const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME};

    // create device
    VkDeviceCreateInfo deviceCI = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCI.pNext = &sync2Features;
    deviceCI.ppEnabledExtensionNames = deviceExtensions;
    deviceCI.enabledExtensionCount = ARRAY_SIZE(deviceExtensions);
    deviceCI.pEnabledFeatures = &deviceFeatures;
    deviceCI.queueCreateInfoCount = deviceQueueCI.size();
    deviceCI.pQueueCreateInfos = deviceQueueCI.data();

    VK_CHECK(vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device));
    volkLoadDevice(device);

    // get queues
    vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
    vkGetDeviceQueue(device, computeQueueIndex, 0, &computeQueue);
}

void VulkanRenderDevice::createAllocator()
{
    VmaVulkanFunctions vmaFunctions = {};
    vmaFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vmaFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    vmaFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    vmaFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    vmaFunctions.vkAllocateMemory = vkAllocateMemory;
    vmaFunctions.vkFreeMemory = vkFreeMemory;
    vmaFunctions.vkMapMemory = vkMapMemory;
    vmaFunctions.vkUnmapMemory = vkUnmapMemory;
    vmaFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    vmaFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    vmaFunctions.vkBindBufferMemory = vkBindBufferMemory;
    vmaFunctions.vkBindImageMemory = vkBindImageMemory;
    vmaFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    vmaFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    vmaFunctions.vkCreateBuffer = vkCreateBuffer;
    vmaFunctions.vkDestroyBuffer = vkDestroyBuffer;
    vmaFunctions.vkCreateImage = vkCreateImage;
    vmaFunctions.vkDestroyImage = vkDestroyImage;
    vmaFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
    vmaFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
    vmaFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
    vmaFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2;
    vmaFunctions.vkBindImageMemory2KHR = vkBindImageMemory2;
    vmaFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;

    VmaAllocatorCreateInfo createInfo = {};
    createInfo.instance = instance;
    createInfo.device = device;
    createInfo.physicalDevice = physicalDevice;
    createInfo.pVulkanFunctions = &vmaFunctions;
    createInfo.vulkanApiVersion = VULKAN_API_VERSION;
    createInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    VK_CHECK(vmaCreateAllocator(&createInfo, &allocator));
}

VkCommandBuffer VulkanRenderDevice::createCommandBuffer(VkCommandBufferLevel level, bool start)
{
    VkCommandBufferAllocateInfo bufferAllocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    bufferAllocInfo.commandPool = commandPool;
    bufferAllocInfo.level = level;
    bufferAllocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VK_CHECK(vkAllocateCommandBuffers(device, &bufferAllocInfo, &commandBuffer));

    if (start) {
        VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    }

    return commandBuffer;
}

void VulkanRenderDevice::flushCommandBuffer(VkCommandBuffer cmd, VkQueue queue, VkCommandPool pool, bool free)
{
    if (cmd == VK_NULL_HANDLE)
        return;

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkSubmitInfo submit = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.pCommandBuffers = &cmd;
    submit.commandBufferCount = 1;

    VkFenceCreateInfo fenceCreateInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};

    VkFence fence = VK_NULL_HANDLE;
    VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));

    VK_CHECK(vkQueueSubmit(queue, 1, &submit, fence));
    VK_CHECK(vkWaitForFences(device, 1, &fence, VK_TRUE, ~0L));
    vkDestroyFence(device, fence, nullptr);

    if (free)
        vkFreeCommandBuffers(device, pool, 1, &cmd);
}