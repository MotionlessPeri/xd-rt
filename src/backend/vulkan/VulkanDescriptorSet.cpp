//
// Created by Frank on 2024/1/15.
//

#include "VulkanDescriptorSet.h"

#include <stdexcept>

#include "VulkanDevice.h"

using namespace xd;
void VulkanDescriptorSet::bindResource(uint32_t binding, const VkDescriptorBufferInfo& bufferInfo)
{
	if (boundBuffers.contains(binding)) {
		throw std::runtime_error{"Resource is already bound!\n"};
	}
	boundBuffers[binding] = bufferInfo;
}

void VulkanDescriptorSet::bindResource(uint32_t binding, const VkDescriptorImageInfo& imageInfo)
{
	if (boundImages.contains(binding)) {
		throw std::runtime_error{"Resource is already bound!\n"};
	}
	boundImages[binding] = imageInfo;
}

void VulkanDescriptorSet::bindResource(const std::string& name,
									   const VkDescriptorBufferInfo& bufferInfo)
{
	bindResource(layoutRef->queryBinding(name), bufferInfo);
}

void VulkanDescriptorSet::bindResource(const std::string& name,
									   const VkDescriptorImageInfo& imageInfo)
{
	bindResource(layoutRef->queryBinding(name), imageInfo);
}

void VulkanDescriptorSet::updateDescriptors()
{
	std::vector<VkWriteDescriptorSet> writes;
	addWriteDescriptorSet(writes, boundBuffers);
	addWriteDescriptorSet(writes, boundImages);
	device->updateDescriptorSets(writes);
	boundBuffers.clear();
	boundImages.clear();
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
