//
// Created by Frank on 2024/1/15.
//

#ifndef XD_RT_VULKANDESCRIPTORPOOL_H
#define XD_RT_VULKANDESCRIPTORPOOL_H
#include "VulkanDescs.h"
#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
namespace xd {

class VulkanDescriptorPool : public VulkanDeviceObject<DescriptorPoolDesc>,
							 public std::enable_shared_from_this<VulkanDescriptorPool> {
public:
	friend class VulkanDevice;
	friend class VulkanDescriptorSetLayout;
	friend class ImguiAppBase;	// TODO: find a better way to encapsulate imgui, maybe in the lib?
	VulkanDescriptorPool() = delete;
	VulkanDescriptorPool(const VulkanDescriptorPool& other) = delete;
	VulkanDescriptorPool(VulkanDescriptorPool&& other) noexcept = delete;
	VulkanDescriptorPool& operator=(const VulkanDescriptorPool& other) = delete;
	VulkanDescriptorPool& operator=(VulkanDescriptorPool&& other) noexcept = delete;
	~VulkanDescriptorPool();

private:
	VulkanDescriptorPool(std::shared_ptr<const VulkanDevice> device,
						 const DescriptorPoolDesc& desc,
						 VkDescriptorPool pool);

	std::shared_ptr<VulkanDescriptorSet> createDescriptorSet(
		std::shared_ptr<const VulkanDescriptorSetLayout> layout) const;
	VkDescriptorPool pool = VK_NULL_HANDLE;
};

}  // namespace xd

#endif	// XD_RT_VULKANDESCRIPTORPOOL_H
