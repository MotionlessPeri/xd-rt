//
// Created by Frank on 2024/2/18.
//

#include "VulkanPipelineBase.h"

#include <utility>
#include "VulkanCommandBuffer.h"
#include "VulkanPipelineLayout.h"
using namespace xd;

VulkanPipelineBase::VulkanPipelineBase(VkPipeline pipeline,
									   std::shared_ptr<VulkanPipelineLayout> layout)
	: pipeline(pipeline), layout(std::move(layout))
{
}

void VulkanPipelineBase::bindPipelineInternal(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
											  VkPipelineBindPoint bindPoint) const
{
	cmdBuffer->bindPipeline(bindPoint, pipeline);
}

void VulkanPipelineBase::bindDescriptorSetsInternal(
	std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
	VkPipelineBindPoint bindPoint,
	uint32_t firstSet,
	const std::vector<std::shared_ptr<VulkanDescriptorSet>>& descSets) const
{
	layout->bindDescriptorSets(std::move(cmdBuffer), bindPoint, firstSet, descSets);
}
