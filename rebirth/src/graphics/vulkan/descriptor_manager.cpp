#include <rebirth/graphics/vulkan/descriptor_manager.h>
#include <rebirth/graphics/vulkan/graphics.h>

namespace vulkan
{

void DescriptorManager::initialize(Graphics &graphics)
{
    std::vector<VkDescriptorPoolSize> poolSizes = {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, // scene data
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES}, // textures
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4}, // materials, lights, joints, vertices
    };

    pool = graphics.createDescriptorPool(poolSizes, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

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
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
        {
            .binding = LIGHTS_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
        {
            .binding = VERTEX_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL,
        },
        {
            .binding = JOINT_MATRICES_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
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

} // namespace vulkan