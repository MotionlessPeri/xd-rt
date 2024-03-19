//
// Created by Frank on 2024/2/18.
//

#include "VulkanComputePipeline.h"
#include "VulkanDevice.h"
using namespace xd;
VulkanComputePipeline::~VulkanComputePipeline()
{
	device->destroyPipeline(pipeline);
}

void VulkanComputePipeline::bindPipeline(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const
{
	bindPipelineInternal(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE);
}

void VulkanComputePipeline::bindDescriptorSets(
	std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
	uint32_t firstSet,
	const std::vector<std::shared_ptr<VulkanDescriptorSet>>& descSets) const
{
	bindDescriptorSetsInternal(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, firstSet, descSets);
}

VulkanComputePipeline::VulkanComputePipeline(std::shared_ptr<const VulkanDevice> device,
											 const VkComputePipelineCreateInfo& desc,
											 VkPipeline pipeline,
											 std::shared_ptr<VulkanPipelineLayout> layout)
	: VulkanDeviceObject(std::move(device), desc), VulkanPipelineBase(pipeline, std::move(layout))
{
}
