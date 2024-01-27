//
// Created by Frank on 2024/1/25.
//

#include "VulkanPipelineLayout.h"
#include <ranges>
#include "VulkanCommandBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"
using namespace xd;
VulkanPipelineLayout::~VulkanPipelineLayout()
{
	device->destroyPipelineLayout(handle);
}

void VulkanPipelineLayout::bindDescriptorSets(
	std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
	uint32_t firstSet,
	const std::vector<std::shared_ptr<VulkanDescriptorSet>>& descSets) const
{
	const auto descSetHandlesView =
		descSets | std::views::transform([](const auto& descSet) { return descSet->set; });
	cmdBuffer->bindDescriptorSets(handle, firstSet,
								  {descSetHandlesView.begin(), descSetHandlesView.end()});
}

VulkanPipelineLayout::VulkanPipelineLayout(std::shared_ptr<const VulkanDevice> device,
										   const PipelineLayoutDesc& desc,
										   VkPipelineLayout handle)
	: VulkanDeviceObject(std::move(device), desc), handle(handle)
{
}
