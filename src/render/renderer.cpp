#include "render/renderer.h"
#include "EASTL/unique_ptr.h"
#include "math/math.h"
#include "render/graphics_types.h"

#ifdef RENDER_API_VULKAN
#include "render/vulkan/vulkan_render_device.h"
#endif

Renderer::Renderer(Application *application)
{
#ifdef RENDER_API_VULKAN
    renderDevice = eastl::make_unique<VulkanRenderDevice>(application);
#endif

    renderGraph = eastl::make_unique<RenderGraph>();

    vec2 windowSize = application->getWindowSize();

    TextureCreateParams textureParmas = {
        .width = (uint32_t)windowSize.x,
        .height = (uint32_t)windowSize.y,
        .arrayLayers = 0,
        .mipLevels = renderDevice->calculateMipLevels(windowSize.x, windowSize.y),
        .sampleCount = 1,
        .usage = (int)TextureUsageFlags::ColorAttachment,
        .format = TextureFormat::R8G8B8A8_SRGB,
    };

    colorTarget = renderDevice->createTexture(textureParmas);
}

Renderer::~Renderer()
{
    renderDevice->destroyTexture(colorTarget);
}