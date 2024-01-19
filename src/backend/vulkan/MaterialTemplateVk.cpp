//
// Created by Frank on 2024/1/15.
//

#include "MaterialTemplateVk.h"

#include <algorithm>
#include <ranges>
#include <unordered_map>
#include "MaterialInstanceVk.h"
#include "VulkanDescriptorSetLayout.h"
using namespace xd;
MaterialTemplateVk::MaterialTemplateVk(
	std::shared_ptr<VulkanDevice> device,
	std::vector<std::shared_ptr<VulkanShader>> shaders,
	std::vector<std::shared_ptr<VulkanDescriptorSetLayout>> set_layouts,
	std::shared_ptr<VulkanGraphicsPipeline> pipeline)
	: device(std::move(device)),
	  shaders(std::move(shaders)),
	  setLayouts(std::move(set_layouts)),
	  pipeline(std::move(pipeline))
{
}

std::shared_ptr<MaterialInstanceVk> MaterialTemplateVk::createInstance() const
{
	return std::make_shared<MaterialInstanceVk>(device, shared_from_this());
}

std::vector<VkDescriptorPoolSize> MaterialTemplateVk::getPoolSizes() const
{
	std::unordered_map<VkDescriptorType, VkDescriptorPoolSize> poolSizes;
	const auto addPoolSize = [&](VkDescriptorType type, uint32_t count) {
		if (poolSizes.contains(type)) {
			poolSizes[type].descriptorCount += count;
		}
		else {
			poolSizes[type] = {type, count};
		}
	};
	for (const auto& setLayout : setLayouts) {
		const auto& bindings = setLayout->getBindings();
		for (const auto& binding : bindings) {
			addPoolSize(binding.descriptorType, binding.descriptorCount);
		}
	}

	const auto keysView = poolSizes | std::views::keys;
	return {keysView.begin(), keysView.end()};
}
