//
// Created by Frank on 2024/1/15.
//

#include "MaterialInstanceVk.h"
#include <numeric>
#include "MaterialTemplateVk.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanDevice.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanPipelineLayout.h"
using namespace xd;
MaterialInstanceVk::MaterialInstanceVk(std::shared_ptr<VulkanDevice> _device,
									   std::shared_ptr<const MaterialTemplateVk> mtl_template)
	: device(std::move(_device)), mtlTemplate(std::move(mtl_template))
{
	DescriptorPoolDesc desc;
	desc.poolSizes = mtlTemplate->getPoolSizes();
	desc.ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	desc.ci.pNext = nullptr;
	desc.ci.flags = 0;
	desc.ci.maxSets = std::accumulate(
		desc.poolSizes.begin(), desc.poolSizes.end(), 0,
		[](const auto& acc, const auto& poolSize) { return acc + poolSize.descriptorCount; });
	desc.ci.poolSizeCount = desc.poolSizes.size();
	desc.ci.pPoolSizes = desc.poolSizes.data();
	pool = device->createDescriptorPool(desc);
	const auto& descSetLayouts = mtlTemplate->getLayouts();
	descriptorSets.resize(descSetLayouts.size());
	std::ranges::transform(descSetLayouts, descriptorSets.begin(), [&](const auto& setLayout) {
		return setLayout->createDescriptorSet(pool);
	});
}

std::shared_ptr<VulkanDescriptorSet> MaterialInstanceVk::queryDescriptorSet(
	const std::string& name) const
{
	const auto index = mtlTemplate->querySetIndex(name);
	return descriptorSets[index];
}

void MaterialInstanceVk::updateDescriptorSets() const
{
	std::ranges::for_each(descriptorSets,
						  [](const auto& descSetPtr) { descSetPtr->updateDescriptors(); });
}

void MaterialInstanceVk::bindDescriptorSets(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const
{
	mtlTemplate->pipeline->bindDescriptorSets(cmdBuffer, 0, descriptorSets);
}
