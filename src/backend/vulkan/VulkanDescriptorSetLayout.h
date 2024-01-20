//
// Created by Frank on 2024/1/15.
//

#ifndef XD_RT_VULKANDESCRIPTORSETLAYOUT_H
#define XD_RT_VULKANDESCRIPTORSETLAYOUT_H
#include <vector>
#include "VulkanDescs.h"
#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
namespace xd {

class VulkanDescriptorSetLayout : public VulkanDeviceObject<DescriptorSetLayoutDesc>,
								  public std::enable_shared_from_this<VulkanDescriptorSetLayout> {
public:
	friend class VulkanDevice;
	friend class VulkanDescriptorPool;
	friend class VulkanGLFWApp;	 // TODO: remove it ASAP when FrameGraph is built
	VulkanDescriptorSetLayout() = delete;
	VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout& other) = delete;
	VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept = delete;
	VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout& other) = delete;
	VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&& other) noexcept = delete;
	~VulkanDescriptorSetLayout();
	std::shared_ptr<VulkanDescriptorSet> createDescriptorSet(
		std::shared_ptr<VulkanDescriptorPool> pool) const;
	const VkDescriptorSetLayoutBinding& getBinding(uint32_t index) const;
	const std::vector<VkDescriptorSetLayoutBinding>& getBindings() const { return desc.bindings; }

private:
	VulkanDescriptorSetLayout(std::shared_ptr<const VulkanDevice> device,
							  const DescriptorSetLayoutDesc& desc,
							  VkDescriptorSetLayout layout);
	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
};

}  // namespace xd

#endif	// XD_RT_VULKANDESCRIPTORSETLAYOUT_H
