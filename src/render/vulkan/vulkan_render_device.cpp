#include "render/vulkan/vulkan_render_device.h"

#include "EASTL/set.h"
#include "EASTL/vector.h"

#include "render/graphics_types.h"
#include "render/vulkan/vulkan_config.h"
#include "render/vulkan/vulkan_helpers.h"
#include "render/vulkan/vulkan_sdl_window_system.h"
#include "render/vulkan/vulkan_swapchain.h"

#include "render/vulkan/vulkan_types.h"
#include "core/logger.h"
#include "core/util.h"
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
    windowSystem = new VulkanSDLWindowSystem(application->getHandle());
#endif

    createInstance();

#ifdef WINDOW_SYSTEM_SDL
    static_cast<VulkanSDLWindowSystem *>(windowSystem)->createSurface(instance, &surface);
#endif

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

    VulkanSwapchainCreateParams swapchainParams;
    swapchainParams.device = device;
    swapchainParams.physicalDevice = physicalDevice;
    swapchainParams.pWindowSystem = windowSystem;
    swapchainParams.surface = surface;
    swapchain.create(swapchainParams);

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

#ifdef ENABLE_VULKAN_PROFILE
    for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        tracyVkCtx[i] = TracyVkContext(physicalDevice, device, graphicsQueue, commandBuffers[i]);
    }
#endif
}

VulkanRenderDevice::~VulkanRenderDevice()
{
    vkDeviceWaitIdle(device);

    delete windowSystem;

    for (VkSemaphore &semaphore : submitSemaphores) {
        vkDestroySemaphore(device, semaphore, nullptr);
    }

    for (unsigned int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, acquireSemaphores[i], nullptr);
        vkDestroyFence(device, finishRenderFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    swapchain.destroy(device);

    vmaDestroyAllocator(allocator);

    vkDestroyDevice(device, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);

#ifdef ENABLE_VULKAN_DEBUG
    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif

    vkDestroyInstance(instance, nullptr);
}

Buffer *VulkanRenderDevice::createBuffer(const BufferCreateParams &params)
{
    assert(params.size > 0);

    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = params.size;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = vulkan::getBufferUsageFlags(params.usage);

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocInfo.priority = 1.0;

    VulkanBuffer *buffer = new VulkanBuffer();
    buffer->size = params.size;
    buffer->usage = params.usage;
    VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer->buffer, &buffer->allocation.handle, &buffer->allocation.info));

    if ((bufferInfo.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkBufferDeviceAddressInfo deviceAddressInfo = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR};
        deviceAddressInfo.buffer = buffer->buffer;
        buffer->address = vkGetBufferDeviceAddress(device, &deviceAddressInfo);
    }

    return buffer;
}

Texture *VulkanRenderDevice::createTexture(const TextureCreateParams &params)
{
    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = vulkan::getImageType(params.type);
    imageInfo.format = vulkan::getFormat(params.format);
    imageInfo.extent.width = params.width;
    imageInfo.extent.height = params.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = params.mipLevels;
    imageInfo.arrayLayers = params.arrayLayers;
    imageInfo.samples = vulkan::getSampleCount(params.sampleCount);
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = vulkan::getImageUsageFlags(params.usage);
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    VulkanTexture *texture = new VulkanTexture();
    assert(texture);
    texture->width = params.width;
    texture->height = params.height;
    texture->layerCount = params.arrayLayers;
    texture->levelCount = params.mipLevels;
    texture->sampleCount = params.sampleCount;
    texture->type = params.type;
    texture->usage = params.usage;
    texture->format = params.format;
    VK_CHECK(vmaCreateImage(allocator, &imageInfo, &allocInfo, &texture->image, &texture->allocation.handle, &texture->allocation.info));

    return texture;
}

Texture *VulkanRenderDevice::createTextureView(const TextureViewCreateParams &params)
{
    VulkanTexture *pViewedTexture = (VulkanTexture *)params.pTexture;
    assert(pViewedTexture);

    VulkanTexture *texture = new VulkanTexture();
    assert(texture);

    texture->pViewedTexture = params.pTexture;
    texture->width = pViewedTexture->width;
    texture->height = pViewedTexture->height;
    texture->layerCount = pViewedTexture->layerCount;
    texture->levelCount = pViewedTexture->levelCount;
    texture->sampleCount = pViewedTexture->sampleCount;
    texture->type = pViewedTexture->type;
    texture->usage = pViewedTexture->usage;
    texture->format = pViewedTexture->format;

    VkImageViewCreateInfo imageViewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    imageViewInfo.image = texture->image;
    imageViewInfo.viewType = vulkan::getImageViewType(texture->type);
    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.format = vulkan::getFormat(texture->format);
    imageViewInfo.subresourceRange = vulkan::getImageSubresourceRange(texture);
    VK_CHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &texture->view));

    return texture;
}

Sampler *VulkanRenderDevice::createSampler(const SamplerCreateParams &params)
{
    VkSamplerCreateInfo createInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    createInfo.minFilter = vulkan::getFilter(params.minFilter);
    createInfo.magFilter = vulkan::getFilter(params.magFilter);
    createInfo.addressModeU = vulkan::getSamplerAddressMode(params.addressModeU);
    createInfo.addressModeV = vulkan::getSamplerAddressMode(params.addressModeV);
    createInfo.addressModeW = vulkan::getSamplerAddressMode(params.addressModeW);
    createInfo.mipmapMode = vulkan::getSamplerMipmapMode(params.mipmapMode);
    createInfo.compareOp = vulkan::getCompareOp(params.compareOp);
    createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    createInfo.maxLod = params.maxLod;

    VulkanSampler *sampler = new VulkanSampler();
    assert(sampler);
    sampler->mipLodBias = params.mipLodBias;
    sampler->minLod = params.minLod;
    sampler->maxLod = params.maxLod;
    sampler->maxAnisotropy = params.maxAnisotropy;
    sampler->magFilter = params.magFilter;
    sampler->minFilter = params.minFilter;
    sampler->mipmapMode = params.mipmapMode;
    sampler->addressModeU = params.addressModeU;
    sampler->addressModeV = params.addressModeV;
    sampler->addressModeW = params.addressModeW;
    sampler->compareOp = params.compareOp;
    VK_CHECK(vkCreateSampler(device, &createInfo, nullptr, &sampler->sampler));

    return sampler;
}

PipelineLayout *VulkanRenderDevice::createPipelineLayout(const PipelineLayoutCreateParams &params)
{
    eastl::vector<VkDescriptorSetLayout> descriptorSetLayouts(params.descriptorSetLayouts.size());

    for (size_t i = 0; i < params.descriptorSetLayouts.size(); i++) {
        const eastl::vector<DescriptorSetLayoutBinding> &bindings = params.descriptorSetLayouts[i].bindings;

        eastl::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings(bindings.size());

        for (size_t j = 0; j < descriptorSetLayoutBindings.size(); j++) {
            descriptorSetLayoutBindings[i].binding = bindings[i].binding;
            descriptorSetLayoutBindings[i].descriptorType = vulkan::getDescriptorType(bindings[i].descriptorType);
            descriptorSetLayoutBindings[i].descriptorCount = bindings[i].descriptorCount;
            descriptorSetLayoutBindings[i].stageFlags = vulkan::getShaderStageFlags(bindings[i].stageMask);
        }

        VkDescriptorSetLayoutCreateInfo createInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        createInfo.bindingCount = descriptorSetLayoutBindings.size();
        createInfo.pBindings = descriptorSetLayoutBindings.data();

        VK_CHECK(vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &descriptorSetLayouts[i]));
    }

    VkPipelineLayoutCreateInfo createInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    createInfo.setLayoutCount = descriptorSetLayouts.size();
    createInfo.pSetLayouts = descriptorSetLayouts.data();

    VulkanPipelineLayout *pipelineLayout = new VulkanPipelineLayout();
    VK_CHECK(vkCreatePipelineLayout(device, &createInfo, nullptr, &pipelineLayout->layout));

    return pipelineLayout;
}

RenderPipeline *VulkanRenderDevice::createRenderPipeline(const RenderPipelineCreateParams &params)
{
    VulkanPipelineLayout *vulkanPipelineLayout = (VulkanPipelineLayout *)params.pPipelineLayout;
    assert(vulkanPipelineLayout);

    eastl::vector<VkPipelineShaderStageCreateInfo> stages;

    VkShaderModule vertexModule = VK_NULL_HANDLE;
    VkShaderModule fragmentModule = VK_NULL_HANDLE;
    VkShaderModule tessellationControlModule = VK_NULL_HANDLE;
    VkShaderModule tessellationEvaluationModule = VK_NULL_HANDLE;

    if (!params.vertexCode.empty()) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleCreateInfo.codeSize = params.vertexCode.size();
        shaderModuleCreateInfo.pCode = params.vertexCode.data();
        VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &vertexModule));

        VkPipelineShaderStageCreateInfo &stage = stages.emplace_back();
        stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage.module = vertexModule;
        stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        stage.pName = "main";
    }

    if (!params.fragmentCode.empty()) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleCreateInfo.codeSize = params.fragmentCode.size();
        shaderModuleCreateInfo.pCode = params.fragmentCode.data();
        VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &fragmentModule));

        VkPipelineShaderStageCreateInfo &stage = stages.emplace_back();
        stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage.module = fragmentModule;
        stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        stage.pName = "main";
    }

    if (!params.tessellationControlCode.empty()) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleCreateInfo.codeSize = params.tessellationControlCode.size();
        shaderModuleCreateInfo.pCode = params.tessellationControlCode.data();
        VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &tessellationControlModule));

        VkPipelineShaderStageCreateInfo &stage = stages.emplace_back();
        stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage.module = tessellationControlModule;
        stage.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        stage.pName = "main";
    }

    if (!params.tessellationEvaluationCode.empty()) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleCreateInfo.codeSize = params.tessellationEvaluationCode.size();
        shaderModuleCreateInfo.pCode = params.tessellationEvaluationCode.data();
        VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &tessellationEvaluationModule));

        VkPipelineShaderStageCreateInfo &stage = stages.emplace_back();
        stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stage.module = tessellationEvaluationModule;
        stage.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        stage.pName = "main";
    }

    VkPipelineVertexInputStateCreateInfo   vertexInputState = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssemblyState.topology = vulkan::getPrimitiveTopology(params.topology);

    VkPipelineTessellationStateCreateInfo tessellationState = {VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO};
    tessellationState.patchControlPoints = params.patchControlPoints;

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = 0.0f;
    viewport.height = 0.0f;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = 0;
    scissor.extent.height = 0;

    VkPipelineViewportStateCreateInfo viewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizationState = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.polygonMode = vulkan::getPolygonMode(params.polygonMode);
    rasterizationState.cullMode = vulkan::getCullMode(params.cullMode);
    rasterizationState.frontFace = vulkan::getFrontFace(params.frontFace);
    rasterizationState.depthBiasEnable = VK_FALSE;
    rasterizationState.depthBiasConstantFactor = 0.0f;
    rasterizationState.depthBiasClamp = 0.0f;
    rasterizationState.depthBiasSlopeFactor = 0.0f;
    rasterizationState.lineWidth = 1.0;

    VkPipelineMultisampleStateCreateInfo multisampleState = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisampleState.rasterizationSamples = vulkan::getSampleCount(params.sampleCount);
    if (deviceFeatures.sampleRateShading) {
        multisampleState.sampleShadingEnable = VK_TRUE;
        multisampleState.minSampleShading = 0.2f;
    } else {
        multisampleState.sampleShadingEnable = VK_FALSE;
    }

    const VkBool32 depthTestEnabled = params.depthCompareOp != CompareOperator::Always;
    const VkBool32 depthWriteEnabled = params.depthWriteEnable;

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depthStencilState.depthTestEnable = depthTestEnabled || depthWriteEnabled;
    depthStencilState.depthWriteEnable = depthWriteEnabled;
    depthStencilState.depthCompareOp = vulkan::getCompareOp(params.depthCompareOp);
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.stencilTestEnable = VK_FALSE; // TODO: stencil
    depthStencilState.front = {}; // TODO: stencil
    depthStencilState.back = {}; // TODO: stencil
    depthStencilState.minDepthBounds = 0.0f;
    depthStencilState.maxDepthBounds = 1.0f;

    const VkBool32 blendEnabled = params.colorBlendOp != BlendOperator::None;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
    colorBlendAttachmentState.blendEnable = blendEnabled;
    colorBlendAttachmentState.srcColorBlendFactor = vulkan::getBlendFactor(params.colorBlendFactorSrc);
    colorBlendAttachmentState.dstColorBlendFactor = vulkan::getBlendFactor(params.colorBlendFactorDst);
    colorBlendAttachmentState.colorBlendOp = vulkan::getBlendOp(params.colorBlendOp);
    colorBlendAttachmentState.srcAlphaBlendFactor = vulkan::getBlendFactor(params.alphaBlendFactorSrc);
    colorBlendAttachmentState.dstAlphaBlendFactor = vulkan::getBlendFactor(params.alphaBlendFactorDst);
    colorBlendAttachmentState.alphaBlendOp = vulkan::getBlendOp(params.alphaBlendOp);
    colorBlendAttachmentState.colorWriteMask = vulkan::getColorComponentFlags(params.colorWriteMask);

    eastl::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(params.renderTargetFormats.size(), colorBlendAttachmentState);

    VkPipelineColorBlendStateCreateInfo colorBlendState = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.attachmentCount = colorBlendAttachments.size();
    colorBlendState.pAttachments = colorBlendAttachments.data();

    eastl::vector<VkDynamicState> dynamicStates;
    if (params.dynamicState & (int)DynamicStateFlags::Viewport) {
        dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    }

    if (params.dynamicState & (int)DynamicStateFlags::Scissor) {
        dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
    }

    VkPipelineDynamicStateCreateInfo dynamicState = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();

    eastl::vector<VkFormat> colorAttachmentFormats(params.renderTargetFormats.size());
    for (const TextureFormat &renderTargetFormat : params.renderTargetFormats) {
        colorAttachmentFormats.push_back(vulkan::getFormat(renderTargetFormat));
    }

    VkPipelineRenderingCreateInfoKHR renderingInfo = {VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR};
    renderingInfo.colorAttachmentCount = colorAttachmentFormats.size();
    renderingInfo.pColorAttachmentFormats = colorAttachmentFormats.data();
    if (depthTestEnabled || depthWriteEnabled)
        renderingInfo.depthAttachmentFormat = vulkan::getFormat(params.depthTargetFormat);

    VkGraphicsPipelineCreateInfo createInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    createInfo.pNext = &renderingInfo;
    createInfo.stageCount = stages.size();
    createInfo.pStages = stages.data();
    createInfo.pVertexInputState = &vertexInputState;
    createInfo.pInputAssemblyState = &inputAssemblyState;
    createInfo.pTessellationState = &tessellationState;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterizationState;
    createInfo.pMultisampleState = &multisampleState;
    createInfo.pDepthStencilState = &depthStencilState;
    createInfo.pColorBlendState = &colorBlendState;
    createInfo.pDynamicState = &dynamicState;
    createInfo.layout = vulkanPipelineLayout->layout;

    VulkanRenderPipeline *renderPipeline = new VulkanRenderPipeline();
    assert(renderPipeline);
    VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &renderPipeline->pipeline));

    return renderPipeline;
}

ComputePipeline *VulkanRenderDevice::createComputePipeline(const ComputePipelineCreateParams &params)
{
    VulkanPipelineLayout *vulkanPipelineLayout = (VulkanPipelineLayout *)params.pPipelineLayout;
    assert(vulkanPipelineLayout);

    VkPipelineShaderStageCreateInfo computeStage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    VkShaderModule computeModule = VK_NULL_HANDLE;

    if (!params.computeCode.empty()) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleCreateInfo.codeSize = params.computeCode.size();
        shaderModuleCreateInfo.pCode = params.computeCode.data();
        VK_CHECK(vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &computeModule));

        computeStage.module = computeModule;
        computeStage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        computeStage.pName = "main";
    }

    assert(computeModule != VK_NULL_HANDLE);

    VkComputePipelineCreateInfo createInfo = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
    createInfo.stage = computeStage;
    createInfo.layout = vulkanPipelineLayout->layout;

    VulkanComputePipeline *computePipeline = new VulkanComputePipeline();
    assert(computePipeline);
    VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &computePipeline->pipeline));

    return computePipeline;
}

void VulkanRenderDevice::destroyBuffer(Buffer *buffer)
{
    if (buffer) {
        VulkanBuffer *vulkanBuffer = static_cast<VulkanBuffer *>(buffer);
        vmaDestroyBuffer(allocator, vulkanBuffer->buffer, vulkanBuffer->allocation.handle);
        delete buffer;
    }
}

void VulkanRenderDevice::destroyTexture(Texture *texture)
{
    if (texture) {
        VulkanTexture *vulkanTexture = static_cast<VulkanTexture *>(texture);

        if (vulkanTexture->view != VK_NULL_HANDLE)
            vkDestroyImageView(device, vulkanTexture->view, nullptr);

        if (vulkanTexture->image != VK_NULL_HANDLE)
            vmaDestroyImage(allocator, vulkanTexture->image, vulkanTexture->allocation.handle);

        delete texture;
    }
}

void VulkanRenderDevice::destroySampler(Sampler *sampler)
{
    if (sampler) {
        VulkanSampler *vulkanSampler = static_cast<VulkanSampler*>(sampler);
    
        if (vulkanSampler->sampler != VK_NULL_HANDLE)
            vkDestroySampler(device, vulkanSampler->sampler, nullptr);

        delete sampler;
    }
}

void VulkanRenderDevice::destroyPipelineLayout(PipelineLayout *layout)
{
    if (layout) {
        VulkanPipelineLayout *vulkanPipelineLayout = static_cast<VulkanPipelineLayout*>(layout);

        if (vulkanPipelineLayout->layout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(device, vulkanPipelineLayout->layout, nullptr);
        
        delete layout;
    }
}

void VulkanRenderDevice::destroyPipeline(RenderPipeline *pipeline)
{
    if (pipeline) {
        VulkanRenderPipeline *vulkanRenderPipeline = static_cast<VulkanRenderPipeline*>(pipeline);

        if (vulkanRenderPipeline->pipeline != VK_NULL_HANDLE)
            vkDestroyPipeline(device, vulkanRenderPipeline->pipeline, nullptr);

        delete pipeline;
    }
}

void VulkanRenderDevice::destroyPipeline(ComputePipeline *pipeline)
{
    if (pipeline) {
        VulkanComputePipeline *vulkanComputePipeline = static_cast<VulkanComputePipeline*>(pipeline);

        if (vulkanComputePipeline->pipeline != VK_NULL_HANDLE)
            vkDestroyPipeline(device, vulkanComputePipeline->pipeline, nullptr);

        delete pipeline;
    }
}

CommandBuffer *VulkanRenderDevice::beginCommandBuffer()
{
    VulkanCommandBuffer *commandBuffer = new VulkanCommandBuffer();

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

void VulkanRenderDevice::submitCommandBuffer(CommandBuffer *commandBuffer)
{
    assert(commandBuffer);

    uint32_t imageIndex = swapchain.getImageIndex();

    VulkanCommandBuffer *vulkanCommandBuffer = new VulkanCommandBuffer();
    VkCommandBuffer cmd = vulkanCommandBuffer->cmd;

    // Command buffer end
    VK_CHECK(vkEndCommandBuffer(cmd));

    // Submit
    VkSubmitInfo submit = {};
    VkPipelineStageFlags stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &acquireSemaphores[currentFrame];
    submit.pWaitDstStageMask = stages;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &submitSemaphores[imageIndex];
    VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submit, finishRenderFences[currentFrame]));

    // Present
    VkResult result = swapchain.present(graphicsQueue, submitSemaphores[imageIndex]);
    if (resizeRequested || result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // recreateSwapchain(); // TODO:
    }

    currentFrame = (currentFrame + 1) % FRAMES_IN_FLIGHT;

    delete commandBuffer;
}

void VulkanRenderDevice::draw(CommandBuffer *commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
    VkCommandBuffer cmd = static_cast<VulkanCommandBuffer*>(commandBuffer)->cmd;
    vkCmdDraw(cmd, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanRenderDevice::drawIndexed(CommandBuffer *commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
    VkCommandBuffer cmd = static_cast<VulkanCommandBuffer*>(commandBuffer)->cmd;
    vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanRenderDevice::bindPipeline(CommandBuffer *commandBuffer, RenderPipeline *pipeline)
{
    VkCommandBuffer cmd = static_cast<VulkanCommandBuffer*>(commandBuffer)->cmd;
    VulkanRenderPipeline *vulkanRenderPipeline = static_cast<VulkanRenderPipeline*>(pipeline);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanRenderPipeline->pipeline);
}

void VulkanRenderDevice::bindPipeline(CommandBuffer *commandBuffer, ComputePipeline *pipeline)
{
    VkCommandBuffer cmd = static_cast<VulkanCommandBuffer*>(commandBuffer)->cmd;
    VulkanComputePipeline *vulkanComputePipeline = static_cast<VulkanComputePipeline*>(pipeline);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, vulkanComputePipeline->pipeline);
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

#ifdef WINDOW_SYSTEM_SDL
    eastl::vector<const char *> instanceExtensions = static_cast<VulkanSDLWindowSystem *>(windowSystem)->getInstanceExtensions();
#endif

#ifdef ENABLE_VULKAN_DEBUG
    instanceExtensions.push_back("VK_EXT_debug_utils");
#endif

    eastl::vector<const char *> instanceLayers;
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

    eastl::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));

    // FIXME: picking first device
    physicalDevice = physicalDevices[0];
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    // find queue indices
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    eastl::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
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
    eastl::set<uint32_t> uniqueQueueIndices = {graphicsQueueIndex, computeQueueIndex};

    eastl::vector<VkDeviceQueueCreateInfo> deviceQueueCI;

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
    VkPhysicalDeviceVulkan12Features features12 = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    features12.runtimeDescriptorArray = VK_TRUE;
    features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    features12.bufferDeviceAddress = VK_TRUE;

    // Dynamic rendering features
    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES};
    dynamicRenderingFeatures.dynamicRendering = VK_TRUE;
    dynamicRenderingFeatures.pNext = &features12;

    const char *deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    // create device
    VkDeviceCreateInfo deviceCI = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    deviceCI.pNext = &dynamicRenderingFeatures;
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