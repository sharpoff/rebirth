#pragma once

#include <deque>
#include <vector>
#include <volk.h>

namespace rebirth::vulkan
{

class DescriptorWriter
{
public:
    void write(uint32_t binding, VkBuffer &buffer, uint32_t size, VkDescriptorType type, uint32_t dstArrayElement);
    void write(uint32_t binding, VkImageView &imageView, VkSampler &sampler, VkImageLayout layout, VkDescriptorType type, uint32_t dstArrayElement);

    void update(VkDevice device, VkDescriptorSet set);
    void clear();

private:
    // to keep infos in the memory save them into deque
    std::deque<VkDescriptorImageInfo> imageInfos;
    std::deque<VkDescriptorBufferInfo> bufferInfos;

    std::vector<VkWriteDescriptorSet> writes;
};

} // namespace rebirth::vulkan