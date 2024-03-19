//
// Created by Frank on 2024/1/13.
//

#ifndef XD_RT_VULKANPIPELINE_H
#define XD_RT_VULKANPIPELINE_H
#include <vector>
#include "VulkanPipelineBase.h"

namespace xd {

class VulkanGraphicsPipeline : public VulkanDeviceObject<GraphicsPipelineDesc>,
							   public VulkanPipelineBase {
public:
	friend class VulkanDevice;
	friend class BasicGraphicApp;  // TODO: remove it ASAP when FrameGraph is built
	VulkanGraphicsPipeline() = delete;
	VulkanGraphicsPipeline(const VulkanGraphicsPipeline& other) = delete;
	VulkanGraphicsPipeline(VulkanGraphicsPipeline&& other) noexcept = delete;
	VulkanGraphicsPipeline& operator=(const VulkanGraphicsPipeline& other) = delete;
	VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline&& other) noexcept = delete;
	~VulkanGraphicsPipeline() override;
	void bindPipeline(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const override;
	void bindDescriptorSets(
		std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
		uint32_t firstSet,
		const std::vector<std::shared_ptr<VulkanDescriptorSet>>& descSets) const override;

private:
	VulkanGraphicsPipeline(std::shared_ptr<const VulkanDevice> device,
						   const GraphicsPipelineDesc& desc,
						   VkPipeline pipeline,
						   std::shared_ptr<VulkanPipelineLayout> layout);
};

}  // namespace xd

#endif	// XD_RT_VULKANPIPELINE_H
