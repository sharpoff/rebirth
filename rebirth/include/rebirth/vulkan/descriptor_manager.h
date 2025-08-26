#pragma once

#include <stdint.h>
#include <volk.h>

namespace rebirth::vulkan
{

class Graphics;

class DescriptorManager
{
public:
    void initialize(Graphics &graphics);
    void destroy(VkDevice device);

    static const size_t maxResources = 16384;
    static const uint32_t sceneDataBinding = 0;
    static const uint32_t texturesBinding = 1;

    VkDescriptorPool &getPool() { return pool; }
    VkDescriptorSet &getSet() { return set; }
    VkDescriptorSetLayout &getSetLayout() { return setLayout; }

private:
    VkDescriptorPool pool;
    VkDescriptorSetLayout setLayout;
    VkDescriptorSet set;
};
} // namespace rebirth::vulkan