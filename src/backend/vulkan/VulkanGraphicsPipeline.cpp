//
// Created by Frank on 2024/1/13.
//

#include "VulkanGraphicsPipeline.h"

#include "VulkanDevice.h"
using namespace xd;
VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
{
	device->destroyPipeline(pipeline);
}

VulkanGraphicsPipeline::VulkanGraphicsPipeline(std::shared_ptr<const VulkanDevice> device,
											   const GraphicsPipelineDesc& desc,
											   VkPipeline pipeline)
	: VulkanDeviceObject(std::move(device), desc), VulkanPipelineBase(pipeline)
{
}
