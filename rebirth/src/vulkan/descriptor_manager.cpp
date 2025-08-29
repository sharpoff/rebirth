#include <rebirth/vulkan/descriptor_manager.h>
#include <rebirth/vulkan/graphics.h>

#include <vector>

namespace rebirth::vulkan
{

void DescriptorManager::initialize(Graphics &graphics)
{
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, // scene data
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_MATERIALS + MAX_LIGHTS}, // materials, lights
    };

    pool =
        graphics.createDescriptorPool(poolSizes, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {
            .binding = SCENE_DATA_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
        {
            .binding = TEXTURES_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = MAX_TEXTURES,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
        {
            .binding = MATERIALS_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = MAX_MATERIALS,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
        {
            .binding = LIGHTS_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = MAX_LIGHTS,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
    };

    setLayout = graphics.createDescriptorSetLayout(bindings.data(), bindings.size(), nullptr);
    set = graphics.createDescriptorSet(pool, setLayout);
}

void DescriptorManager::destroy(VkDevice device)
{
    vkDestroyDescriptorSetLayout(device, setLayout, nullptr);
    vkDestroyDescriptorPool(device, pool, nullptr);
}

} // namespace rebirth::vulkan