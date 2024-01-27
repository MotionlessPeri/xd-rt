//
// Created by Frank on 2024/1/13.
//

#include "VulkanGraphicsPipeline.h"

#include <utility>

#include "VulkanCommandBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"
#include "VulkanPipelineLayout.h"
using namespace xd;

VulkanPipelineBase::VulkanPipelineBase(VkPipeline pipeline,
									   std::shared_ptr<VulkanPipelineLayout> layout)
	: pipeline(pipeline), layout(std::move(layout))
{
}

void VulkanPipelineBase::bindDescriptorSets(
	std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
	uint32_t firstSet,
	const std::vector<std::shared_ptr<VulkanDescriptorSet>>& descSets) const
{
	layout->bindDescriptorSets(std::move(cmdBuffer), firstSet, descSets);
}

void VulkanPipelineBase::bindPipelineInternal(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
											  VkPipelineBindPoint bindPoint) const
{
	cmdBuffer->bindPipeline(bindPoint, pipeline);
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
	device->destroyPipeline(pipeline);
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(std::shared_ptr<const VulkanDevice> device,
											   const GraphicsPipelineDesc& desc,
											   VkPipeline pipeline,
											   std::shared_ptr<VulkanPipelineLayout> layout)
	: VulkanDeviceObject(std::move(device), desc), VulkanPipelineBase(pipeline, std::move(layout))
{
}

void VulkanGraphicsPipeline::bindPipeline(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const
{
	bindPipelineInternal(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
}
