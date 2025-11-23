#pragma once

#include "EASTL/deque.h"
#include "EASTL/vector.h"

#include <vulkan/vulkan.h>

class VulkanDescriptorSetWriter
{
public:
    void write(uint32_t binding, VkBuffer &buffer, uint32_t size, VkDescriptorType type, uint32_t dstArrayElement);
    void write(uint32_t binding, VkImageView &imageView, VkSampler &sampler, VkImageLayout layout, VkDescriptorType type, uint32_t dstArrayElement);

    void update(VkDevice device, VkDescriptorSet set);
    void clear();

private:
    // to keep infos in the memory save them into deque
    eastl::deque<VkDescriptorImageInfo>  imageInfos;
    eastl::deque<VkDescriptorBufferInfo> bufferInfos;

    eastl::vector<VkWriteDescriptorSet> writes;
};