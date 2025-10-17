#pragma once

#include <stdint.h>
#include <volk.h>

namespace vulkan
{

class Graphics;

static constexpr uint32_t MAX_TEXTURES = 16384;

static constexpr uint32_t SCENE_DATA_BINDING = 0;
static constexpr uint32_t TEXTURES_BINDING = 1;
static constexpr uint32_t MATERIALS_BINDING = 2;
static constexpr uint32_t LIGHTS_BINDING = 3;
static constexpr uint32_t JOINT_MATRICES_BINDING = 4;
static constexpr uint32_t VERTEX_BINDING = 5;

class DescriptorManager
{
public:
    void initialize(Graphics &graphics);
    void destroy(VkDevice device);

    VkDescriptorPool &getPool() { return pool; }
    VkDescriptorSet &getSet() { return set; }
    VkDescriptorSetLayout &getSetLayout() { return setLayout; }

private:
    VkDescriptorPool pool;
    VkDescriptorSetLayout setLayout;
    VkDescriptorSet set;
};
} // namespace vulkan