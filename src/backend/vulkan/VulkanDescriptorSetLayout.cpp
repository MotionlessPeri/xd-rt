//
// Created by Frank on 2024/1/15.
//

#include "VulkanDescriptorSetLayout.h"

#include "VulkanDescriptorPool.h"
#include "VulkanDevice.h"
using namespace xd;
VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
	device->destroyDescriptorSetLayout(layout);
}

std::shared_ptr<VulkanDescriptorSet> VulkanDescriptorSetLayout::createDescriptorSet(
	std::shared_ptr<VulkanDescriptorPool> pool) const
{
	return pool->createDescriptorSet(shared_from_this());
}

const VkDescriptorSetLayoutBinding& VulkanDescriptorSetLayout::getBinding(uint32_t index) const
{
	return desc.bindings[index];
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(std::shared_ptr<const VulkanDevice> device,
													 const DescriptorSetLayoutDesc& desc,
													 VkDescriptorSetLayout layout)
	: VulkanDeviceObject(std::move(device), desc), layout(layout)
{
}
