#include <rebirth/vulkan/descriptor_manager.h>
#include <rebirth/vulkan/graphics.h>

#include <vector>

namespace rebirth::vulkan
{

void DescriptorManager::initialize(Graphics &graphics)
{
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxResources},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxResources},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, maxResources},
    };

    pool = graphics.createDescriptorPool(poolSizes, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

    // XXX: should i hardcode it like that?
    std::vector<VkDescriptorSetLayoutBinding> bindings = {
        {
            .binding = sceneDataBinding,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
        {
            .binding = texturesBinding,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = maxResources,
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