//
// Created by Frank on 2024/1/15.
//

#include "MaterialTemplateVk.h"
#include <algorithm>
#include <ranges>
#include <stdexcept>
#include <unordered_map>
#include "MaterialInstanceVk.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanPipelineLayout.h"
using namespace xd;
MaterialTemplateVk::MaterialTemplateVk(std::shared_ptr<VulkanDevice> device,
									   std::vector<std::shared_ptr<VulkanShader>> shaders,
									   std::unordered_map<std::string, uint32_t> set_name_to_index,
									   const std::shared_ptr<VulkanPipelineBase>& pipeline)
	: device(std::move(device)),
	  shaders(std::move(shaders)),
	  setNameToIndex(std::move(set_name_to_index)),
	  pipeline(pipeline)
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
	for (const auto& setLayout : pipeline->getLayout()->getDesc().setLayouts) {
		const auto& bindings = setLayout->getBindings();
		for (const auto& binding : bindings) {
			addPoolSize(binding.descriptorType, binding.descriptorCount);
		}
	}

	const auto keysView = poolSizes | std::views::values;
	return {keysView.begin(), keysView.end()};
}

uint32_t MaterialTemplateVk::querySetIndex(const std::string& name) const
{
	if (const auto it = setNameToIndex.find(name); it != setNameToIndex.end()) {
		return it->second;
	}
	throw std::invalid_argument{"invalid name.\n"};
}

void MaterialTemplateVk::bindPipeline(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const
{
	pipeline->bindPipeline(cmdBuffer);
}
