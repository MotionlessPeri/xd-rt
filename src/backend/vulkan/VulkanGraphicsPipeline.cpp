//
// Created by Frank on 2024/1/13.
//

#include "VulkanGraphicsPipeline.h"

#include "VulkanCommandBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"
using namespace xd;

void VulkanPipelineBase::bindDescriptorSets(
	std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
	uint32_t firstSet,
	const std::vector<std::shared_ptr<VulkanDescriptorSet>>& descSets) const
{
	const auto descSetHandlesView =
		descSets | std::views::transform([](const auto& descSet) { return descSet->set; });
	cmdBuffer->bindDescriptorSets(layout, firstSet,
								  {descSetHandlesView.begin(), descSetHandlesView.end()});
}

VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
	device->destroyPipeline(pipeline);
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(std::shared_ptr<const VulkanDevice> device,
											   const GraphicsPipelineDesc& desc,
											   VkPipeline pipeline)
	: VulkanDeviceObject(std::move(device), desc), VulkanPipelineBase(pipeline, desc.ci.layout)
{
}
