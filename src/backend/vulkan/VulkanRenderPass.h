//
// Created by Frank on 2024/1/16.
//

#ifndef XD_RT_VULKANRENDERPASS_H
#define XD_RT_VULKANRENDERPASS_H
#include <memory>
#include <vector>
#include "VulkanDescs.h"
#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {

class VulkanSubpass : public VulkanDeviceObject<SubpassDesc> {
public:
	friend class VulkanRenderPass;
	friend class FrameGraphPass;
	VulkanSubpass() = delete;
	VulkanSubpass(const VulkanSubpass& other) = delete;
	VulkanSubpass(VulkanSubpass&& other) noexcept = delete;
	VulkanSubpass& operator=(const VulkanSubpass& other) = delete;
	VulkanSubpass& operator=(VulkanSubpass&& other) noexcept = delete;
	std::shared_ptr<VulkanFrameBuffer> createFrameBuffer(VkFramebufferCreateInfo&& ci) const;
	std::shared_ptr<VulkanGraphicsPipeline> createGraphicsPipeline(
		GraphicsPipelineDesc&& desc) const;

private:
	VulkanSubpass(std::shared_ptr<const VulkanDevice> device,
				  const SubpassDesc& desc,
				  std::shared_ptr<VulkanRenderPass> render_pass,
				  uint32_t index);
	std::shared_ptr<VulkanRenderPass> renderPass;
	uint32_t index;
};

class VulkanRenderPass : public VulkanDeviceObject<RenderPassDesc>,
						 public std::enable_shared_from_this<VulkanRenderPass> {
public:
	friend class VulkanDevice;
	friend class VulkanSubpass;
	friend class VulkanGLFWApp;	 // TODO: remove it ASAP when FrameGraph is built
	VulkanRenderPass() = delete;
	VulkanRenderPass(const VulkanRenderPass& other) = delete;
	VulkanRenderPass(VulkanRenderPass&& other) noexcept = delete;
	VulkanRenderPass& operator=(const VulkanRenderPass& other) = delete;
	VulkanRenderPass& operator=(VulkanRenderPass&& other) noexcept = delete;
	~VulkanRenderPass();
	std::shared_ptr<VulkanFrameBuffer> createFrameBuffer(VkFramebufferCreateInfo&& ci) const;
	std::vector<std::shared_ptr<VulkanSubpass>> getSubpasses() const { return subpasses; }

private:
	VulkanRenderPass(std::shared_ptr<const VulkanDevice> device,
					 const RenderPassDesc& desc,
					 VkRenderPass pass);
	std::vector<std::shared_ptr<VulkanSubpass>> subpasses;
	VkRenderPass pass = VK_NULL_HANDLE;
};

}  // namespace xd

#endif	// XD_RT_VULKANRENDERPASS_H
