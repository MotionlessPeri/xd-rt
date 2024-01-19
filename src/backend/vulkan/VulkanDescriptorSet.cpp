//
// Created by Frank on 2024/1/15.
//

#include "VulkanDescriptorSet.h"

#include <stdexcept>

#include "VulkanDevice.h"

using namespace xd;
void VulkanDescriptorSet::bindResource(uint32_t index, const VkDescriptorBufferInfo& bufferInfo)
{
	if (boundBuffers.contains(index)) {
		throw std::runtime_error{"Resource is already bound!\n"};
	}
	boundBuffers[index] = bufferInfo;
}

void VulkanDescriptorSet::bindResource(uint32_t index, const VkDescriptorImageInfo& imageInfo)
{
	if (boundImages.contains(index)) {
		throw std::runtime_error{"Resource is already bound!\n"};
	}
	boundImages[index] = imageInfo;
}

void VulkanDescriptorSet::updateDescriptor()
{
	std::vector<VkWriteDescriptorSet> writes;
	addWriteDescriptorSet(writes, boundBuffers);
	addWriteDescriptorSet(writes, boundImages);
	device->updateDescriptorSets(writes);
}

VulkanDescriptorSet::VulkanDescriptorSet(
	std::shared_ptr<const VulkanDevice> device,
	const VkDescriptorSetAllocateInfo& desc,
	std::shared_ptr<const VulkanDescriptorSetLayout> layout_ref,
	std::shared_ptr<const VulkanDescriptorPool> pool_ref,
	VkDescriptorSet set)
	: VulkanDeviceObject(std::move(device), desc),
	  layoutRef(std::move(layout_ref)),
	  poolRef(std::move(pool_ref)),
	  set(set)
{
}
