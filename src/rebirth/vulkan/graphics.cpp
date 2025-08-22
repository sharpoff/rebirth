#include <cmath>
#include <set>
#include <stdio.h>

#include <rebirth/util/common.h>
#include <rebirth/util/logger.h>
#include <rebirth/vulkan/graphics.h>
#include <rebirth/vulkan/swapchain.h>
#include <rebirth/vulkan/util.h>

#include "backend/imgui_impl_sdl3.h"
#include "backend/imgui_impl_vulkan.h"
#include "imgui.h"

#include <stb_image.h>

const int MAX_IMGUI_TEXTURES = 1000;

namespace rebirth::vulkan
{

VkBool32 VKAPI_PTR
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
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

    printf("[%s][%s] %s\n", type, severity, pCallbackData->pMessage);

    return VK_FALSE;
}

Graphics::Graphics(SDL_Window *window)
{
    assert(window);
    this->window = window;

    VK_CHECK(volkInitialize());
    createInstance();
    volkLoadInstance(instance);

#ifndef NDEBUG
    createDebugMessenger();
#endif

    createSurface();

    createDevice();
    volkLoadDevice(device);

    sampleCount = getMaxSampleCount();

    createAllocator(VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT);

    swapchain.init(window, *this);

    createCommandPool();
    createCommandBuffers();

    createSyncPrimitives();

    initializeImGui();
}

Graphics::~Graphics()
{
    vkDeviceWaitIdle(device);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(device, imGuiDesctiptorPool, nullptr);

    for (size_t i = 0; i < swapchain.getImagesCount(); i++) {
        vkDestroySemaphore(device, submitSemaphores[i], nullptr);
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

#ifndef NDEBUG
    vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif

    vkDestroyInstance(instance, nullptr);
}

void Graphics::createInstance()
{
    VkApplicationInfo appCI = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appCI.apiVersion = VK_API_VERSION_1_4;
    appCI.pEngineName = "Engine";
    appCI.pApplicationName = "Application";
    appCI.engineVersion = 0;
    appCI.applicationVersion = 0;

    uint32_t extensionCount = 0;
    const char *const *extensionNames = SDL_Vulkan_GetInstanceExtensions(&extensionCount);
    assert(extensionNames && extensionCount > 0);

    std::vector<const char *> instanceExtensions(extensionNames, extensionNames + extensionCount);
#ifndef NDEBUG
    instanceExtensions.push_back("VK_EXT_debug_utils");
#endif

    std::vector<const char *> instanceLayers;
#ifndef NDEBUG
    instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    VkInstanceCreateInfo instanceCI = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    instanceCI.pApplicationInfo = &appCI;
    instanceCI.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
    instanceCI.ppEnabledLayerNames = instanceLayers.data();
    instanceCI.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    instanceCI.ppEnabledExtensionNames = instanceExtensions.data();

#ifndef NDEBUG
    VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messengerInfo.pfnUserCallback = debugCallback;
    messengerInfo.pUserData = nullptr;

    instanceCI.pNext = &messengerInfo;
#endif

    VK_CHECK(vkCreateInstance(&instanceCI, nullptr, &instance));
}

void Graphics::createDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT messengerInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    messengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messengerInfo.pfnUserCallback = debugCallback;
    messengerInfo.pUserData = nullptr;

    VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &messengerInfo, nullptr, &debugMessenger));
}

void Graphics::createSurface() { assert(SDL_Vulkan_CreateSurface(window, instance, nullptr, &surface)); }

void Graphics::createDevice()
{
    uint32_t physicalDeviceCount = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr));
    assert(physicalDeviceCount > 0);

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    VK_CHECK(vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()));

    // FIXME: picking first device
    physicalDevice = physicalDevices[0];
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    // printDeviceProperties(physicalDevice);

    findQueueIndices();

    std::set<uint32_t> uniqueQueueIndices = {graphicsQueueIndex, presentQueueIndex, computeQueueIndex};

    std::vector<VkDeviceQueueCreateInfo> deviceQueueCI;
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
    features12.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    features12.bufferDeviceAddress = VK_TRUE;

    // Dynamic rendering features
    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES};
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

    // get queues
    vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentQueueIndex, 0, &presentQueue);
    vkGetDeviceQueue(device, computeQueueIndex, 0, &computeQueue);
}

void Graphics::printDeviceProperties(VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties propertices;
    vkGetPhysicalDeviceProperties(physicalDevice, &propertices);

    std::cout << "===============================\n";
    std::cout << "Physical device propertices:\n";
    std::cout << "    apiVersion: " << propertices.apiVersion << std::endl;
    std::cout << "    driverVersion: " << propertices.driverVersion << std::endl;
    std::cout << "    vendorID: " << propertices.vendorID << std::endl;
    std::cout << "    deviceID: " << propertices.deviceID << std::endl;
    std::cout << "    deviceType: ";

    switch (propertices.deviceType) {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            std::cout << "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU\n";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            std::cout << "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU\n";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            std::cout << "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU\n";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            std::cout << "VK_PHYSICAL_DEVICE_TYPE_CPU\n";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_OTHER:
            std::cout << "VK_PHYSICAL_DEVICE_TYPE_OTHER\n";
            break;
        case VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM:
            std::cout << "VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM\n";
            break;
    }
    std::cout << "    pipelineCacheUUID: " << propertices.pipelineCacheUUID << std::endl;

    std::cout << "\n";

    std::cout << "===============================\n";
    std::cout << "Physical device limits:\n";
    std::cout << "    limits.maxImageDimension1D: " << propertices.limits.maxImageDimension1D << std::endl;
    std::cout << "    limits.maxImageDimension2D: " << propertices.limits.maxImageDimension2D << std::endl;
    std::cout << "    limits.maxImageDimension3D: " << propertices.limits.maxImageDimension3D << std::endl;
    std::cout << "    limits.maxImageDimensionCube: " << propertices.limits.maxImageDimensionCube << std::endl;
    std::cout << "    limits.maxImageArrayLayers: " << propertices.limits.maxImageArrayLayers << std::endl;
    std::cout << "    limits.maxTexelBufferElements: " << propertices.limits.maxTexelBufferElements << std::endl;
    std::cout << "    limits.maxUniformBufferRange: " << propertices.limits.maxUniformBufferRange << std::endl;
    std::cout << "    limits.maxStorageBufferRange: " << propertices.limits.maxStorageBufferRange << std::endl;
    std::cout << "    limits.maxPushConstantsSize: " << propertices.limits.maxPushConstantsSize << std::endl;
    std::cout << "    limits.maxMemoryAllocationCount: " << propertices.limits.maxMemoryAllocationCount << std::endl;
    std::cout << "    limits.maxSamplerAllocationCount: " << propertices.limits.maxSamplerAllocationCount << std::endl;
    std::cout << "    limits.bufferImageGranularity: " << propertices.limits.bufferImageGranularity << std::endl;
    std::cout << "    limits.sparseAddressSpaceSize: " << propertices.limits.sparseAddressSpaceSize << std::endl;
    std::cout << "    limits.maxBoundDescriptorSets: " << propertices.limits.maxBoundDescriptorSets << std::endl;
    std::cout << "    limits.maxPerStageDescriptorSamplers: " << propertices.limits.maxPerStageDescriptorSamplers << std::endl;
    std::cout << "    limits.maxPerStageDescriptorUniformBuffers: " << propertices.limits.maxPerStageDescriptorUniformBuffers << std::endl;
    std::cout << "    limits.maxPerStageDescriptorStorageBuffers: " << propertices.limits.maxPerStageDescriptorStorageBuffers << std::endl;
    std::cout << "    limits.maxPerStageDescriptorSampledImages: " << propertices.limits.maxPerStageDescriptorSampledImages << std::endl;
    std::cout << "    limits.maxPerStageDescriptorStorageImages: " << propertices.limits.maxPerStageDescriptorStorageImages << std::endl;
    std::cout << "    limits.maxPerStageDescriptorInputAttachments: " << propertices.limits.maxPerStageDescriptorInputAttachments << std::endl;
    std::cout << "    limits.maxPerStageResources: " << propertices.limits.maxPerStageResources << std::endl;
    std::cout << "    limits.maxDescriptorSetSamplers: " << propertices.limits.maxDescriptorSetSamplers << std::endl;
    std::cout << "    limits.maxDescriptorSetUniformBuffers: " << propertices.limits.maxDescriptorSetUniformBuffers << std::endl;
    std::cout << "    limits.maxDescriptorSetUniformBuffersDynamic: " << propertices.limits.maxDescriptorSetUniformBuffersDynamic << std::endl;
    std::cout << "    limits.maxDescriptorSetStorageBuffers: " << propertices.limits.maxDescriptorSetStorageBuffers << std::endl;
    std::cout << "    limits.maxDescriptorSetStorageBuffersDynamic: " << propertices.limits.maxDescriptorSetStorageBuffersDynamic << std::endl;
    std::cout << "    limits.maxDescriptorSetSampledImages: " << propertices.limits.maxDescriptorSetSampledImages << std::endl;
    std::cout << "    limits.maxDescriptorSetStorageImages: " << propertices.limits.maxDescriptorSetStorageImages << std::endl;
    std::cout << "    limits.maxDescriptorSetInputAttachments: " << propertices.limits.maxDescriptorSetInputAttachments << std::endl;
    std::cout << "    limits.maxVertexInputAttributes: " << propertices.limits.maxVertexInputAttributes << std::endl;
    std::cout << "    limits.maxVertexInputBindings: " << propertices.limits.maxVertexInputBindings << std::endl;
    std::cout << "    limits.maxVertexInputAttributeOffset: " << propertices.limits.maxVertexInputAttributeOffset << std::endl;
    std::cout << "    limits.maxVertexInputBindingStride: " << propertices.limits.maxVertexInputBindingStride << std::endl;
    std::cout << "    limits.maxVertexOutputComponents: " << propertices.limits.maxVertexOutputComponents << std::endl;
    std::cout << "    limits.maxTessellationGenerationLevel: " << propertices.limits.maxTessellationGenerationLevel << std::endl;
    std::cout << "    limits.maxTessellationPatchSize: " << propertices.limits.maxTessellationPatchSize << std::endl;
    std::cout << "    limits.maxTessellationControlPerVertexInputComponents: " << propertices.limits.maxTessellationControlPerVertexInputComponents << std::endl;
    std::cout << "    limits.maxTessellationControlPerVertexOutputComponents: " << propertices.limits.maxTessellationControlPerVertexOutputComponents << std::endl;
    std::cout << "    limits.maxTessellationControlPerPatchOutputComponents: " << propertices.limits.maxTessellationControlPerPatchOutputComponents << std::endl;
    std::cout << "    limits.maxTessellationControlTotalOutputComponents: " << propertices.limits.maxTessellationControlTotalOutputComponents << std::endl;
    std::cout << "    limits.maxTessellationEvaluationInputComponents: " << propertices.limits.maxTessellationEvaluationInputComponents << std::endl;
    std::cout << "    limits.maxTessellationEvaluationOutputComponents: " << propertices.limits.maxTessellationEvaluationOutputComponents << std::endl;
    std::cout << "    limits.maxGeometryShaderInvocations: " << propertices.limits.maxGeometryShaderInvocations << std::endl;
    std::cout << "    limits.maxGeometryInputComponents: " << propertices.limits.maxGeometryInputComponents << std::endl;
    std::cout << "    limits.maxGeometryOutputComponents: " << propertices.limits.maxGeometryOutputComponents << std::endl;
    std::cout << "    limits.maxGeometryOutputVertices: " << propertices.limits.maxGeometryOutputVertices << std::endl;
    std::cout << "    limits.maxGeometryTotalOutputComponents: " << propertices.limits.maxGeometryTotalOutputComponents << std::endl;
    std::cout << "    limits.maxFragmentInputComponents: " << propertices.limits.maxFragmentInputComponents << std::endl;
    std::cout << "    limits.maxFragmentOutputAttachments: " << propertices.limits.maxFragmentOutputAttachments << std::endl;
    std::cout << "    limits.maxFragmentDualSrcAttachments: " << propertices.limits.maxFragmentDualSrcAttachments << std::endl;
    std::cout << "    limits.maxFragmentCombinedOutputResources: " << propertices.limits.maxFragmentCombinedOutputResources << std::endl;
    std::cout << "    limits.maxComputeSharedMemorySize: " << propertices.limits.maxComputeSharedMemorySize << std::endl;
    std::cout << "    limits.maxComputeWorkGroupCount[0]: " << propertices.limits.maxComputeWorkGroupCount[0]
              << ", limits.maxComputeWorkGroupCount[1]: " << propertices.limits.maxComputeWorkGroupCount[1]
              << ", limits.maxComputeWorkGroupCount[2]: " << propertices.limits.maxComputeWorkGroupCount[2] << std::endl;
    std::cout << "    limits.maxComputeWorkGroupInvocations: " << propertices.limits.maxComputeWorkGroupInvocations << std::endl;
    std::cout << "    limits.maxComputeWorkGroupSize[0]: " << propertices.limits.maxComputeWorkGroupSize[0] << ", limits.maxComputeWorkGroupSize[1]: " << propertices.limits.maxComputeWorkGroupSize[1]
              << ", limits.maxComputeWorkGroupSize[2]: " << propertices.limits.maxComputeWorkGroupSize[2] << std::endl;
    std::cout << "    limits.subPixelPrecisionBits: " << propertices.limits.subPixelPrecisionBits << std::endl;
    std::cout << "    limits.subTexelPrecisionBits: " << propertices.limits.subTexelPrecisionBits << std::endl;
    std::cout << "    limits.mipmapPrecisionBits: " << propertices.limits.mipmapPrecisionBits << std::endl;
    std::cout << "    limits.maxDrawIndexedIndexValue: " << propertices.limits.maxDrawIndexedIndexValue << std::endl;
    std::cout << "    limits.maxDrawIndirectCount: " << propertices.limits.maxDrawIndirectCount << std::endl;
    std::cout << "    limits.maxSamplerLodBias: " << propertices.limits.maxSamplerLodBias << std::endl;
    std::cout << "    limits.maxSamplerAnisotropy: " << propertices.limits.maxSamplerAnisotropy << std::endl;
    std::cout << "    limits.maxViewports: " << propertices.limits.maxViewports << std::endl;
    std::cout << "    limits.maxViewportDimensions[0]: " << propertices.limits.maxViewportDimensions[0] << ", limits.maxViewportDimensions[1]: " << propertices.limits.maxViewportDimensions[1]
              << std::endl;
    std::cout << "    limits.viewportBoundsRange[0]: " << propertices.limits.viewportBoundsRange[0] << ", limits.viewportBoundsRange[1]: " << propertices.limits.viewportBoundsRange[1] << std::endl;
    std::cout << "    limits.viewportSubPixelBits: " << propertices.limits.viewportSubPixelBits << std::endl;
    std::cout << "    limits.minMemoryMapAlignment: " << propertices.limits.minMemoryMapAlignment << std::endl;
    std::cout << "    limits.minTexelBufferOffsetAlignment: " << propertices.limits.minTexelBufferOffsetAlignment << std::endl;
    std::cout << "    limits.minUniformBufferOffsetAlignment: " << propertices.limits.minUniformBufferOffsetAlignment << std::endl;
    std::cout << "    limits.minStorageBufferOffsetAlignment: " << propertices.limits.minStorageBufferOffsetAlignment << std::endl;
    std::cout << "    limits.minTexelOffset: " << propertices.limits.minTexelOffset << std::endl;
    std::cout << "    limits.maxTexelOffset: " << propertices.limits.maxTexelOffset << std::endl;
    std::cout << "    limits.minTexelGatherOffset: " << propertices.limits.minTexelGatherOffset << std::endl;
    std::cout << "    limits.maxTexelGatherOffset: " << propertices.limits.maxTexelGatherOffset << std::endl;
    std::cout << "    limits.minInterpolationOffset: " << propertices.limits.minInterpolationOffset << std::endl;
    std::cout << "    limits.maxInterpolationOffset: " << propertices.limits.maxInterpolationOffset << std::endl;
    std::cout << "    limits.subPixelInterpolationOffsetBits: " << propertices.limits.subPixelInterpolationOffsetBits << std::endl;
    std::cout << "    limits.maxFramebufferWidth: " << propertices.limits.maxFramebufferWidth << std::endl;
    std::cout << "    limits.maxFramebufferHeight: " << propertices.limits.maxFramebufferHeight << std::endl;
    std::cout << "    limits.maxFramebufferLayers: " << propertices.limits.maxFramebufferLayers << std::endl;
    std::cout << "    limits.framebufferColorSampleCounts: " << propertices.limits.framebufferColorSampleCounts << std::endl;
    std::cout << "    limits.framebufferDepthSampleCounts: " << propertices.limits.framebufferDepthSampleCounts << std::endl;
    std::cout << "    limits.framebufferStencilSampleCounts: " << propertices.limits.framebufferStencilSampleCounts << std::endl;
    std::cout << "    limits.framebufferNoAttachmentsSampleCounts: " << propertices.limits.framebufferNoAttachmentsSampleCounts << std::endl;
    std::cout << "    limits.maxColorAttachments: " << propertices.limits.maxColorAttachments << std::endl;
    std::cout << "    limits.sampledImageColorSampleCounts: " << propertices.limits.sampledImageColorSampleCounts << std::endl;
    std::cout << "    limits.sampledImageIntegerSampleCounts: " << propertices.limits.sampledImageIntegerSampleCounts << std::endl;
    std::cout << "    limits.sampledImageDepthSampleCounts: " << propertices.limits.sampledImageDepthSampleCounts << std::endl;
    std::cout << "    limits.sampledImageStencilSampleCounts: " << propertices.limits.sampledImageStencilSampleCounts << std::endl;
    std::cout << "    limits.storageImageSampleCounts: " << propertices.limits.storageImageSampleCounts << std::endl;
    std::cout << "    limits.maxSampleMaskWords: " << propertices.limits.maxSampleMaskWords << std::endl;
    std::cout << "    limits.timestampComputeAndGraphics: " << propertices.limits.timestampComputeAndGraphics << std::endl;
    std::cout << "    limits.timestampPeriod: " << propertices.limits.timestampPeriod << std::endl;
    std::cout << "    limits.maxClipDistances: " << propertices.limits.maxClipDistances << std::endl;
    std::cout << "    limits.maxCullDistances: " << propertices.limits.maxCullDistances << std::endl;
    std::cout << "    limits.maxCombinedClipAndCullDistances: " << propertices.limits.maxCombinedClipAndCullDistances << std::endl;
    std::cout << "    limits.discreteQueuePriorities: " << propertices.limits.discreteQueuePriorities << std::endl;
    std::cout << "    limits.pointSizeRange[0]: " << propertices.limits.pointSizeRange[0] << ", limits.pointSizeRange[1]: " << propertices.limits.pointSizeRange[1] << std::endl;
    std::cout << "    limits.lineWidthRange[0]: " << propertices.limits.lineWidthRange[0] << ", limits.lineWidthRange[1]: " << propertices.limits.lineWidthRange[1] << std::endl;
    std::cout << "    limits.pointSizeGranularity: " << propertices.limits.pointSizeGranularity << std::endl;
    std::cout << "    limits.lineWidthGranularity: " << propertices.limits.lineWidthGranularity << std::endl;
    std::cout << "    limits.strictLines: " << propertices.limits.strictLines << std::endl;
    std::cout << "    limits.standardSampleLocations: " << propertices.limits.standardSampleLocations << std::endl;
    std::cout << "    limits.optimalBufferCopyOffsetAlignment: " << propertices.limits.optimalBufferCopyOffsetAlignment << std::endl;
    std::cout << "    limits.optimalBufferCopyRowPitchAlignment: " << propertices.limits.optimalBufferCopyRowPitchAlignment << std::endl;
    std::cout << "    limits.nonCoherentAtomSize: " << propertices.limits.nonCoherentAtomSize << std::endl;

    std::cout << "\n";

    std::cout << "===============================\n";
    std::cout << "Physical device sparse properties:\n";
    std::cout << "    sparseProperties.residencyStandard2DBlockShape: " << (propertices.sparseProperties.residencyStandard2DBlockShape ? "true" : "false") << std::endl;
    std::cout << "    sparseProperties.residencyStandard2DMultisampleBlockShape: " << (propertices.sparseProperties.residencyStandard2DMultisampleBlockShape ? "true" : "false") << std::endl;
    std::cout << "    sparseProperties.residencyStandard3DBlockShape: " << (propertices.sparseProperties.residencyStandard3DBlockShape ? "true" : "false") << std::endl;
    std::cout << "    sparseProperties.residencyAlignedMipSize: " << (propertices.sparseProperties.residencyAlignedMipSize ? "true" : "false") << std::endl;
    std::cout << "    sparseProperties.residencyNonResidentStrict: " << (propertices.sparseProperties.residencyNonResidentStrict ? "true" : "false") << std::endl;
}

void Graphics::findQueueIndices()
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    VkBool32 presentSupport = VK_FALSE;

    uint32_t i = 0;
    for (VkQueueFamilyProperties queueFamily : queueFamilies) {
        if (graphicsQueueIndex == UINT32_MAX && queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            graphicsQueueIndex = i;

        if (presentQueueIndex == UINT32_MAX) {
            VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport));
            if (presentSupport)
                presentQueueIndex = i;
        }

        if (computeQueueIndex == UINT32_MAX && queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
            computeQueueIndex = i;
        i++;
    }

    assert(graphicsQueueIndex != UINT32_MAX && presentQueueIndex != UINT32_MAX && computeQueueIndex != UINT32_MAX);
}

VkSampleCountFlagBits Graphics::getMaxSampleCount()
{
    VkSampleCountFlags supportedSampleCount = std::min(deviceProperties.limits.framebufferColorSampleCounts, deviceProperties.limits.framebufferDepthSampleCounts);
    if (supportedSampleCount & VK_SAMPLE_COUNT_64_BIT)
        return VK_SAMPLE_COUNT_64_BIT;
    if (supportedSampleCount & VK_SAMPLE_COUNT_32_BIT)
        return VK_SAMPLE_COUNT_32_BIT;
    if (supportedSampleCount & VK_SAMPLE_COUNT_16_BIT)
        return VK_SAMPLE_COUNT_16_BIT;
    if (supportedSampleCount & VK_SAMPLE_COUNT_8_BIT)
        return VK_SAMPLE_COUNT_8_BIT;
    if (supportedSampleCount & VK_SAMPLE_COUNT_4_BIT)
        return VK_SAMPLE_COUNT_4_BIT;

    return VK_SAMPLE_COUNT_1_BIT;
}

void Graphics::createAllocator(VmaAllocatorCreateFlags flags)
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
    createInfo.vulkanApiVersion = VK_API_VERSION_1_4;
    createInfo.flags = flags;

    VK_CHECK(vmaCreateAllocator(&createInfo, &allocator));
}

void Graphics::createCommandPool()
{
    VkCommandPoolCreateInfo commandPoolCI = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    commandPoolCI.queueFamilyIndex = graphicsQueueIndex;
    commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(device, &commandPoolCI, nullptr, &commandPool));
}

VkCommandBuffer Graphics::createCommandBuffer(VkCommandBufferLevel level, bool start)
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

void Graphics::flushCommandBuffer(VkCommandBuffer cmd, VkQueue queue, VkCommandPool pool, bool free)
{
    if (cmd == VK_NULL_HANDLE)
        return;

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkSubmitInfo submit = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit.pCommandBuffers = &cmd;
    submit.commandBufferCount = 1;

    VkFence fence = createFence();
    VK_CHECK(vkQueueSubmit(queue, 1, &submit, fence));
    VK_CHECK(vkWaitForFences(device, 1, &fence, VK_TRUE, ~0L));
    vkDestroyFence(device, fence, nullptr);

    if (free)
        vkFreeCommandBuffers(device, pool, 1, &cmd);
}

void Graphics::createCommandBuffers()
{
    VkCommandBufferAllocateInfo bufferAllocInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    bufferAllocInfo.commandPool = commandPool;
    bufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    bufferAllocInfo.commandBufferCount = commandBuffers.size();

    VK_CHECK(vkAllocateCommandBuffers(device, &bufferAllocInfo, commandBuffers.data()));
}

void Graphics::createSyncPrimitives()
{
    submitSemaphores.resize(swapchain.getImagesCount());
    for (size_t i = 0; i < submitSemaphores.size(); i++) {
        submitSemaphores[i] = createSemaphore();
        vulkan::setDebugName(device, (uint64_t)submitSemaphores[i], VK_OBJECT_TYPE_SEMAPHORE, "submit semaphore " + std::to_string(i));
    }

    for (unsigned int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        acquireSemaphores[i] = createSemaphore();
        finishRenderFences[i] = createFence(VK_FENCE_CREATE_SIGNALED_BIT);

        vulkan::setDebugName(device, (uint64_t)acquireSemaphores[i], VK_OBJECT_TYPE_SEMAPHORE, "acquire semaphore " + std::to_string(i));
        vulkan::setDebugName(device, (uint64_t)finishRenderFences[i], VK_OBJECT_TYPE_FENCE, "finish render fence " + std::to_string(i));
    }
}

void Graphics::recreateSwapchain()
{
    resizeRequested = false;

    util::logInfo("Recreating swapchain");
    vkDeviceWaitIdle(device);

    swapchain.destroy(device);

    for (unsigned int i = 0; i < FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, acquireSemaphores[i], nullptr);
        vkDestroyFence(device, finishRenderFences[i], nullptr);
    }

    for (auto &semaphore : submitSemaphores) {
        vkDestroySemaphore(device, semaphore, nullptr);
    }

    swapchain.init(window, *this);

    createSyncPrimitives();
}

void Graphics::initializeImGui()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Create descriptor pool
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_IMGUI_TEXTURES},
    };
    imGuiDesctiptorPool = createDescriptorPool(poolSizes, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

    VkFormat colorFormat = VK_FORMAT_B8G8R8A8_SRGB; // TODO: change hardcoded format

    VkPipelineRenderingCreateInfoKHR pipelineRenderingCI = {VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR};
    pipelineRenderingCI.colorAttachmentCount = 1;
    pipelineRenderingCI.pColorAttachmentFormats = &colorFormat;
    pipelineRenderingCI.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.ApiVersion = VK_API_VERSION_1_4;
    initInfo.Instance = instance;
    initInfo.PhysicalDevice = physicalDevice;
    initInfo.Device = device;
    initInfo.QueueFamily = graphicsQueueIndex;
    initInfo.Queue = graphicsQueue;
    initInfo.DescriptorPool = imGuiDesctiptorPool;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = 2;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.Allocator = nullptr;
    initInfo.CheckVkResultFn = nullptr;
    initInfo.RenderPass = nullptr;
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineRenderingCreateInfo = pipelineRenderingCI;

    ImGui_ImplVulkan_Init(&initInfo);
}

void Graphics::uploadBuffer(Buffer &buffer, void *data, VkDeviceSize size)
{
    if (size <= 0) {
        util::logError("Cannot upload buffer - size is 0.");
        return;
    }

    Buffer staging;
    createBuffer(&staging, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    memcpy(staging.info.pMappedData, data, size);
    VK_CHECK(vmaFlushAllocation(allocator, staging.allocation, 0, VK_WHOLE_SIZE));

    // command buffer start
    VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    // copy
    VkBufferCopy copyRegion = {0, 0, size};
    vkCmdCopyBuffer(copyCmd, staging.buffer, buffer.buffer, 1, &copyRegion);

    // submit
    flushCommandBuffer(copyCmd, graphicsQueue, commandPool, true);

    destroyBuffer(&staging);
}

VkCommandBuffer Graphics::beginCommandBuffer()
{
    VK_CHECK(vkWaitForFences(device, 1, &finishRenderFences[currentFrame], VK_TRUE, ~0ull));
    VK_CHECK(vkResetFences(device, 1, &finishRenderFences[currentFrame]));

    VkResult result = swapchain.acquireNextImage(device, acquireSemaphores[currentFrame]);
    if (resizeRequested || result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return VK_NULL_HANDLE;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        util::logError("Failed to acquire swapchain image.");
        exit(EXIT_FAILURE);
    }

    VkCommandBuffer cmd = commandBuffers[currentFrame];
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    // Command buffer begin
    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

    return cmd;
}

void Graphics::submitCommandBuffer(VkCommandBuffer cmd)
{
    uint32_t imageIndex = swapchain.getImageIndex();

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
    VkResult result = swapchain.present(presentQueue, submitSemaphores[imageIndex]);
    if (resizeRequested || result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        recreateSwapchain();

    currentFrame = (currentFrame + 1) % FRAMES_IN_FLIGHT;
}

void Graphics::createImage(
    Image *image,
    uint32_t width,
    uint32_t height,
    VkImageUsageFlags usage,
    VkFormat format,
    VkImageViewType viewType,
    VkImageAspectFlags aspect,
    VkSampleCountFlagBits samples,
    bool generateMipMaps
)
{
    if (!image) {
        util::logWarn("Cannot create image - pointer is null.\n");
        return;
    }

    bool isCubemap = viewType == VK_IMAGE_VIEW_TYPE_CUBE;

    image->mipLevels = generateMipMaps ? static_cast<uint32_t>(floor(log2(std::max(width, height))) + 1) : 1;

    VkImageCreateInfo imageInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
    imageInfo.mipLevels = image->mipLevels;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = samples;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (isCubemap) {
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.arrayLayers = 6;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    VK_CHECK(vmaCreateImage(allocator, &imageInfo, &allocInfo, &image->image, &image->allocation, &image->info));

    VkImageViewCreateInfo imageViewInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    imageViewInfo.image = image->image;
    imageViewInfo.viewType = viewType;
    imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewInfo.format = format;
    imageViewInfo.subresourceRange = {.aspectMask = aspect, .baseMipLevel = 0, .levelCount = image->mipLevels, .baseArrayLayer = 0, .layerCount = (isCubemap ? 6u : 1u)};

    VK_CHECK(vkCreateImageView(device, &imageViewInfo, nullptr, &image->view));

    image->sampler = createSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, isCubemap ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER : VK_SAMPLER_ADDRESS_MODE_REPEAT, image->mipLevels);
}

void Graphics::createImageFromFile(Image *image, std::filesystem::path path)
{
    if (!image) {
        util::logWarn("Cannot create image - pointer is null.\n");
        return;
    }

    unsigned char *pixels = stbi_load(path.c_str(), &image->width, &image->height, &image->channels, STBI_rgb_alpha);
    if (!pixels) {
        util::logError("Failed to load texture: ", path);
        return;
    }

    createImage(
        image, image->width, image->height, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_VIEW_TYPE_2D,
        VK_IMAGE_ASPECT_COLOR_BIT, VK_SAMPLE_COUNT_1_BIT, true
    );

    uint32_t size = image->width * image->height * STBI_rgb_alpha;

    Buffer staging;
    createBuffer(&staging, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    memcpy(staging.info.pMappedData, pixels, size);
    stbi_image_free(pixels);

    // command buffer start
    VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    // transition image to transfer
    VkImageMemoryBarrier barrier0 = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image->image,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}
    };

    vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier0);

    // copy
    VkBufferImageCopy copyRegion = {};
    copyRegion.imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1};
    copyRegion.imageExtent = {static_cast<uint32_t>(image->width), static_cast<uint32_t>(image->height), 1};

    vkCmdCopyBufferToImage(copyCmd, staging.buffer, image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    // transition image to transfer src for read during blit
    VkImageMemoryBarrier barrier1 = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image->image,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}
    };

    vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier1);

    // submit
    flushCommandBuffer(copyCmd, graphicsQueue, commandPool, true);

    destroyBuffer(&staging);

    generateMipmaps(image);
}

void Graphics::createCubemapImage(Image *image, std::filesystem::path dir, VkFormat format)
{
    const int CUBE_FACES_COUNT = 6;

    if (!image) {
        util::logWarn("Cannot create cubemap image - pointer is null.\n");
        return;
    }

    std::array<std::string, CUBE_FACES_COUNT> paths = {"right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg"};

    std::vector<unsigned char *> imagePixels(CUBE_FACES_COUNT);
    for (uint32_t i = 0; i < CUBE_FACES_COUNT; i++) {
        auto path = dir / paths[i];

        imagePixels[i] = stbi_load(path.c_str(), &image->width, &image->height, &image->channels, STBI_rgb_alpha);
        if (!imagePixels[i]) {
            util::logError("Failed to load image: ", path);
            return;
        }
    }

    uint32_t size = image->width * image->height * CUBE_FACES_COUNT * STBI_rgb_alpha;
    uint32_t layerSize = size / CUBE_FACES_COUNT;

    createImage(image, image->width, image->height, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, format, VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_COLOR_BIT, getSampleCount());

    Buffer staging;
    createBuffer(&staging, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

    // copy image pixels into staging buffer
    for (uint32_t i = 0; i < CUBE_FACES_COUNT; i++)
        memcpy(static_cast<unsigned char *>(staging.info.pMappedData) + (layerSize * i), imagePixels[i], layerSize);

    // create temporary command buffer
    VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    // transition image to transfer
    VkImageMemoryBarrier barrier0 = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image->image,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = CUBE_FACES_COUNT}
    };

    vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier0);

    // copy
    std::vector<VkBufferImageCopy> copyRegions(CUBE_FACES_COUNT);
    for (uint32_t i = 0; i < CUBE_FACES_COUNT; i++) {
        copyRegions[i].imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, i, 1};
        copyRegions[i].imageExtent = {static_cast<uint32_t>(image->width), static_cast<uint32_t>(image->height), 1};
        copyRegions[i].bufferOffset = i * layerSize;
    }

    vkCmdCopyBufferToImage(copyCmd, staging.buffer, image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copyRegions.size(), copyRegions.data());

    // transition image to fragment shader
    VkImageMemoryBarrier barrier1 = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image->image,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = CUBE_FACES_COUNT}
    };

    vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier1);

    // submit
    flushCommandBuffer(copyCmd, graphicsQueue, commandPool, true);

    destroyBuffer(&staging);
}

void Graphics::generateMipmaps(Image *image)
{
    if (!image) {
        util::logWarn("Cannot generate mipmaps for image - pointer is null.\n");
        return;
    }

    // command buffer start
    VkCommandBuffer blitCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    // copy mips from n-1 to n
    for (uint32_t i = 1; i < image->mipLevels; i++) {
        VkImageBlit blit{};

        // src
        blit.srcSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = i - 1, .layerCount = 1};
        blit.srcOffsets[1].x = static_cast<int32_t>(image->width >> (i - 1));
        blit.srcOffsets[1].y = static_cast<int32_t>(image->height >> (i - 1));
        blit.srcOffsets[1].z = 1;

        // dst
        blit.dstSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = i, .layerCount = 1};
        blit.dstOffsets[1].x = static_cast<int32_t>(image->width >> i);
        blit.dstOffsets[1].y = static_cast<int32_t>(image->height >> i);
        blit.dstOffsets[1].z = 1;

        VkImageSubresourceRange subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = i, .levelCount = 1, .layerCount = 1};

        // transition mip level to transfer dst
        VkImageMemoryBarrier barrier0 = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image->image,
            .subresourceRange = subresourceRange
        };

        vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier0);

        // blit from previous mip level
        vkCmdBlitImage(blitCmd, image->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        // transition mip level to transfer src
        VkImageMemoryBarrier barrier1 = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image->image,
            .subresourceRange = subresourceRange
        };

        vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier1);
    }

    // all mip layers are in TRANSFER_SRC, so transition all to SHADER_READ
    VkImageMemoryBarrier barrier1 = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image->image,
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = image->mipLevels, .baseArrayLayer = 0, .layerCount = 1}
    };

    vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier1);

    // submit
    flushCommandBuffer(blitCmd, graphicsQueue, commandPool, true);
}

void Graphics::createBuffer(Buffer *buffer, uint32_t size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage)
{
    assert(size > 0);

    VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferInfo.size = size;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.usage = usage;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memUsage;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocInfo.priority = 1.0;

    VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer->buffer, &buffer->allocation, &buffer->info));

    if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkBufferDeviceAddressInfo deviceAddressInfo = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO_KHR};
        deviceAddressInfo.buffer = buffer->buffer;
        buffer->address = vkGetBufferDeviceAddress(device, &deviceAddressInfo);
    }

    buffer->size = size;
}

void Graphics::destroyImage(Image *image)
{
    if (!image)
        return;

    vmaDestroyImage(allocator, image->image, image->allocation);
    vkDestroyImageView(device, image->view, nullptr);
    vkDestroySampler(device, image->sampler, nullptr);
}

void Graphics::destroyBuffer(Buffer *buffer)
{
    if (!buffer)
        return;

    vmaDestroyBuffer(allocator, buffer->buffer, buffer->allocation);
}

VkImageView Graphics::createImageView(VkImage image, VkImageViewType viewType, VkFormat format, VkImageSubresourceRange subresourceRange)
{
    VkImageViewCreateInfo imageViewCI = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    imageViewCI.image = image;
    imageViewCI.viewType = viewType;
    imageViewCI.format = format;
    imageViewCI.subresourceRange = subresourceRange;
    imageViewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    VkImageView imageView = VK_NULL_HANDLE;
    VK_CHECK(vkCreateImageView(device, &imageViewCI, nullptr, &imageView));

    return imageView;
}

VkSampler Graphics::createSampler(VkFilter minFilter, VkFilter magFilter, VkSamplerAddressMode samplerMode, float maxLod)
{
    // TODO: add anisotropy feature
    VkSamplerCreateInfo createInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    createInfo.magFilter = minFilter;
    createInfo.minFilter = magFilter;
    createInfo.addressModeU = samplerMode;
    createInfo.addressModeV = samplerMode;
    createInfo.addressModeW = samplerMode;
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    createInfo.compareOp = VK_COMPARE_OP_NEVER;
    createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    createInfo.maxLod = maxLod;

    VkSampler sampler;
    VK_CHECK(vkCreateSampler(device, &createInfo, nullptr, &sampler));

    return sampler;
}

VkSemaphore Graphics::createSemaphore(VkSemaphoreCreateFlags flags)
{
    VkSemaphoreCreateInfo createInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    createInfo.flags = flags;

    VkSemaphore semaphore;
    VK_CHECK(vkCreateSemaphore(device, &createInfo, nullptr, &semaphore));

    return semaphore;
}

VkFence Graphics::createFence(VkFenceCreateFlags flags)
{
    VkFenceCreateInfo createInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    createInfo.flags = flags;

    VkFence fence;
    VK_CHECK(vkCreateFence(device, &createInfo, nullptr, &fence));

    return fence;
}

VkDescriptorPool Graphics::createDescriptorPool(std::vector<VkDescriptorPoolSize> poolSizes, VkDescriptorPoolCreateFlags flags)
{
    VkDescriptorPoolCreateInfo createInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    createInfo.flags = flags;
    createInfo.maxSets = 0;
    for (auto &poolSize : poolSizes)
        createInfo.maxSets += poolSize.descriptorCount;

    createInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    createInfo.pPoolSizes = poolSizes.data();

    VkDescriptorPool descriptorPool;
    VK_CHECK(vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool));

    return descriptorPool;
}

VkDescriptorSetLayout Graphics::createDescriptorSetLayout(VkDescriptorSetLayoutBinding *bindings, uint32_t bindingCount, VkDescriptorBindingFlags *bindingFlags)
{
    VkDescriptorSetLayoutCreateInfo layoutCI = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
    layoutCI.bindingCount = bindingCount;
    layoutCI.pBindings = bindings;

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsCI = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO};
    if (bindingFlags) {
        flagsCI.pBindingFlags = bindingFlags;
        flagsCI.bindingCount = bindingCount;
        layoutCI.pNext = &flagsCI;
        layoutCI.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    }

    VkDescriptorSetLayout layout;
    VK_CHECK(vkCreateDescriptorSetLayout(device, &layoutCI, nullptr, &layout));

    return layout;
}

VkDescriptorSet Graphics::createDescriptorSet(VkDescriptorPool pool, VkDescriptorSetLayout layout)
{
    VkDescriptorSetAllocateInfo allocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    allocateInfo.descriptorPool = pool;
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &layout;

    VkDescriptorSet set;
    VK_CHECK(vkAllocateDescriptorSets(device, &allocateInfo, &set));
    return set;
}

VkPipelineLayout Graphics::createPipelineLayout(VkDescriptorSetLayout *setLayout, VkPushConstantRange *pushConstant)
{
    VkPipelineLayoutCreateInfo layoutInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    if (setLayout) {
        layoutInfo.setLayoutCount = 1;
        layoutInfo.pSetLayouts = setLayout;
    }
    if (pushConstant) {
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = pushConstant;
    }

    VkPipelineLayout layout;
    vkCreatePipelineLayout(device, &layoutInfo, nullptr, &layout);
    return layout;
}

} // namespace rebirth::vulkan
