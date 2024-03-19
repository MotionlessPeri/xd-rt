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

const VkDescriptorSetLayoutBinding& VulkanDescriptorSetLayout::getBinding(uint32_t binding) const
{
	return desc.bindings[bindingToIndex.at(binding)];
}

const VkDescriptorSetLayoutBinding& VulkanDescriptorSetLayout::getBinding(
	const std::string& name) const
{
	return desc.bindings[bindingToIndex.at(nameToBinding.at(name))];
}

uint32_t VulkanDescriptorSetLayout::queryBinding(const std::string& name) const
{
	return nameToBinding.at(name);
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(
	std::shared_ptr<const VulkanDevice> device,
	const DescriptorSetLayoutDesc& desc,
	VkDescriptorSetLayout layout,
	std::unordered_map<std::string, uint32_t> nameToBinding)
	: VulkanDeviceObject(std::move(device), desc),
	  layout(layout),
	  nameToBinding(std::move(nameToBinding))
{
	for (const auto i : std::views::iota(0ull, desc.bindings.size())) {
		const auto& binding = desc.bindings[i];
		bindingToIndex[binding.binding] = i;
	}
}
