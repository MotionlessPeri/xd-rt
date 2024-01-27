//
// Created by Frank on 2024/1/13.
//

#ifndef XD_RT_VULKANPIPELINE_H
#define XD_RT_VULKANPIPELINE_H
#include <vector>
#include "VulkanDescs.h"
#include "VulkanDeviceObject.h"

namespace xd {

class VulkanPipelineBase {
public:
	VulkanPipelineBase(VkPipeline pipeline, std::shared_ptr<VulkanPipelineLayout> layout);

	VulkanPipelineBase(const VulkanPipelineBase& other) = delete;
	VulkanPipelineBase(VulkanPipelineBase&& other) noexcept = delete;
	VulkanPipelineBase& operator=(const VulkanPipelineBase& other) = delete;
	VulkanPipelineBase& operator=(VulkanPipelineBase&& other) noexcept = delete;
	void bindDescriptorSets(
		std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
		uint32_t firstSet,
		const std::vector<std::shared_ptr<VulkanDescriptorSet>>& descSets) const;
	std::shared_ptr<VulkanPipelineLayout> getLayout() const { return layout; }

protected:
	void bindPipelineInternal(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
							  VkPipelineBindPoint bindPoint) const;
	VkPipeline pipeline = VK_NULL_HANDLE;
	std::shared_ptr<VulkanPipelineLayout> layout = nullptr;
};

class VulkanGraphicsPipeline : public VulkanDeviceObject<GraphicsPipelineDesc>,
							   public VulkanPipelineBase {
public:
	friend class VulkanDevice;
	friend class VulkanGLFWApp;	 // TODO: remove it ASAP when FrameGraph is built
	VulkanGraphicsPipeline() = delete;
	VulkanGraphicsPipeline(const VulkanGraphicsPipeline& other) = delete;
	VulkanGraphicsPipeline(VulkanGraphicsPipeline&& other) noexcept = delete;
	VulkanGraphicsPipeline& operator=(const VulkanGraphicsPipeline& other) = delete;
	VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline&& other) noexcept = delete;
	~VulkanGraphicsPipeline();

	VulkanGraphicsPipeline(std::shared_ptr<const VulkanDevice> device,
						   const GraphicsPipelineDesc& desc,
						   VkPipeline pipeline,
						   std::shared_ptr<VulkanPipelineLayout> layout);
	void bindPipeline(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const;
};

}  // namespace xd

#endif	// XD_RT_VULKANPIPELINE_H
