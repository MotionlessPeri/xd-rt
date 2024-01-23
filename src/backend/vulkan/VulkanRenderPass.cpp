//
// Created by Frank on 2024/1/16.
//

#include "VulkanRenderPass.h"

#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
using namespace xd;

std::shared_ptr<VulkanGraphicsPipeline> VulkanSubpass::createGraphicsPipeline(
	GraphicsPipelineDesc&& desc) const
{
	desc.ci.renderPass = renderPass->pass;
	desc.ci.subpass = index;
	return device->createGraphicsPipeline(std::move(desc));
}

VulkanSubpass::VulkanSubpass(std::shared_ptr<const VulkanDevice> device,
							 const SubpassDesc& desc,
							 std::shared_ptr<VulkanRenderPass> render_pass,
							 uint32_t _index)
	: VulkanDeviceObject(std::move(device), desc), renderPass(std::move(render_pass)), index(_index)
{
}

VulkanRenderPass::~VulkanRenderPass()
{
	device->destroyRenderPass(pass);
}

std::shared_ptr<VulkanFrameBuffer> VulkanRenderPass::createFrameBuffer(
	VkFramebufferCreateInfo&& ci) const
{
	ci.renderPass = pass;
	return device->createFrameBuffer(ci);
}

VulkanRenderPass::VulkanRenderPass(std::shared_ptr<const VulkanDevice> _device,
								   const RenderPassDesc& _desc,
								   VkRenderPass _pass)
	: VulkanDeviceObject(std::move(_device), _desc), pass(_pass)
{
	subpasses.reserve(desc.subpasses.size());
	std::shared_ptr<VulkanRenderPass> sharedInCtor{this, [](const VulkanRenderPass* ptr) {}};
	// Note: we can use enumerate_view in c++23
	for (const auto i : std::views::iota(0ull, desc.subpasses.size())) {
		subpasses.emplace_back(std::shared_ptr<VulkanSubpass>{
			new VulkanSubpass{device, desc.subpasses[i], shared_from_this(), (uint32_t)i}});
	}
}
