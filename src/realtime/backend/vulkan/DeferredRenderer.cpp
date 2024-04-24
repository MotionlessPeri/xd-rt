//
// Created by Frank on 2024/4/24.
//

#include "DeferredRenderer.h"
using namespace xd;
const std::string DEFERRED_PASS_NAME = "DeferredPass";
const std::string GBUFFER_SUBPASS_NAME = "GBufferSubpass";
DeferredRenderer::DeferredRenderer(std::shared_ptr<VulkanDevice> pDevice)
	: device(std::move(pDevice)), builder(device)
{
	auto& renderPass = builder.addGraphicsPass(DEFERRED_PASS_NAME);
	auto& gBufferSubpass = renderPass.addSubpass(
		GBUFFER_SUBPASS_NAME,
		[](FGResourceList& resources, std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
		   const std::vector<std::shared_ptr<VulkanSemaphore>>& waitingSemaphores) {

		});
}
