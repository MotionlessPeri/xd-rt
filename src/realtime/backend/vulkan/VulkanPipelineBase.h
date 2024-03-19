//
// Created by Frank on 2024/2/18.
//

#ifndef XD_RT_VULKANPIPELINEBASE_H
#define XD_RT_VULKANPIPELINEBASE_H
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
	virtual ~VulkanPipelineBase() = default;
	std::shared_ptr<VulkanPipelineLayout> getLayout() const { return layout; }
	virtual void bindPipeline(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const = 0;
	virtual void bindDescriptorSets(
		std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
		uint32_t firstSet,
		const std::vector<std::shared_ptr<VulkanDescriptorSet>>& descSets) const = 0;

protected:
	void bindPipelineInternal(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
							  VkPipelineBindPoint bindPoint) const;
	void bindDescriptorSetsInternal(
		std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
		VkPipelineBindPoint bindPoint,
		uint32_t firstSet,
		const std::vector<std::shared_ptr<VulkanDescriptorSet>>& descSets) const;
	VkPipeline pipeline = VK_NULL_HANDLE;
	std::shared_ptr<VulkanPipelineLayout> layout = nullptr;
};

}  // namespace xd

#endif	// XD_RT_VULKANPIPELINEBASE_H
