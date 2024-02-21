//
// Created by Frank on 2024/2/18.
//

#ifndef XD_RT_VULKANCOMPUTEPIPELINE_H
#define XD_RT_VULKANCOMPUTEPIPELINE_H
#include "VulkanPipelineBase.h"
namespace xd {

class VulkanComputePipeline : public VulkanDeviceObject<VkComputePipelineCreateInfo>,
							  public VulkanPipelineBase {
public:
	friend class VulkanDevice;
	friend class BasicComputeApp;
	VulkanComputePipeline() = delete;
	VulkanComputePipeline(const VulkanComputePipeline& other) = delete;
	VulkanComputePipeline(VulkanComputePipeline&& other) noexcept = delete;
	VulkanComputePipeline& operator=(const VulkanComputePipeline& other) = delete;
	VulkanComputePipeline& operator=(VulkanComputePipeline&& other) noexcept = delete;
	~VulkanComputePipeline() override;
	void bindPipeline(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const override;
	void bindDescriptorSets(
		std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
		uint32_t firstSet,
		const std::vector<std::shared_ptr<VulkanDescriptorSet>>& descSets) const override;

private:
	VulkanComputePipeline(std::shared_ptr<const VulkanDevice> device,
						  const VkComputePipelineCreateInfo& desc,
						  VkPipeline pipeline,
						  std::shared_ptr<VulkanPipelineLayout> layout);
};

}  // namespace xd

#endif	// XD_RT_VULKANCOMPUTEPIPELINE_H
