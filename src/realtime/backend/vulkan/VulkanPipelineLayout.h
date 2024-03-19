//
// Created by Frank on 2024/1/25.
//

#ifndef XD_RT_VULKANPIPELINELAYOUT_H
#define XD_RT_VULKANPIPELINELAYOUT_H
#include "VulkanDescs.h"
#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {
class VulkanPipelineLayout : public VulkanDeviceObject<PipelineLayoutDesc> {
public:
	friend class VulkanDevice;
	VulkanPipelineLayout() = delete;
	VulkanPipelineLayout(const VulkanPipelineLayout& other) = delete;
	VulkanPipelineLayout(VulkanPipelineLayout&& other) noexcept = delete;
	VulkanPipelineLayout& operator=(const VulkanPipelineLayout& other) = delete;
	VulkanPipelineLayout& operator=(VulkanPipelineLayout&& other) noexcept = delete;
	~VulkanPipelineLayout();
	void bindDescriptorSets(
		std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
		VkPipelineBindPoint bindPoint,
		uint32_t firstSet, const std::vector<std::shared_ptr<VulkanDescriptorSet>>& descSets) const;

private:
	VulkanPipelineLayout(std::shared_ptr<const VulkanDevice> device,
						 const PipelineLayoutDesc& desc,
						 VkPipelineLayout handle);

	VkPipelineLayout handle = VK_NULL_HANDLE;
};
}  // namespace xd

#endif	// XD_RT_VULKANPIPELINELAYOUT_H
