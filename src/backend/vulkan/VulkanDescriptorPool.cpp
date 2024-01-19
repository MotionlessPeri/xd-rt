//
// Created by Frank on 2024/1/15.
//

#include "VulkanDescriptorPool.h"

#include "VulkanDescriptorSetLayout.h"
#include "VulkanDevice.h"

namespace xd {
VulkanDescriptorPool::~VulkanDescriptorPool()
{
	device->destroyDescriptorPool(pool);
}

VulkanDescriptorPool::VulkanDescriptorPool(std::shared_ptr<const VulkanDevice> device,
										   const VkDescriptorPoolCreateInfo& desc,
										   VkDescriptorPool pool)
	: VulkanDeviceObject(std::move(device), desc), pool(pool)
{
}

std::shared_ptr<VulkanDescriptorSet> VulkanDescriptorPool::createDescriptorSet(
	std::shared_ptr<const VulkanDescriptorSetLayout> layout) const
{
	VkDescriptorSetAllocateInfo ai;
	ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	ai.descriptorPool = pool;
	ai.descriptorSetCount = 1;
	ai.pSetLayouts = &layout->layout;
	return device->createDescriptorSet(ai, layout, shared_from_this());
}
}  // namespace xd