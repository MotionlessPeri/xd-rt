//
// Created by Frank on 2024/1/15.
//

#ifndef XD_RT_VULKANDESCRIPTORSETLAYOUT_H
#define XD_RT_VULKANDESCRIPTORSETLAYOUT_H
#include <string>
#include <unordered_map>
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
	friend class BasicGraphicApp;  // TODO: remove it ASAP when FrameGraph is built
	VulkanDescriptorSetLayout() = delete;
	VulkanDescriptorSetLayout(const VulkanDescriptorSetLayout& other) = delete;
	VulkanDescriptorSetLayout(VulkanDescriptorSetLayout&& other) noexcept = delete;
	VulkanDescriptorSetLayout& operator=(const VulkanDescriptorSetLayout& other) = delete;
	VulkanDescriptorSetLayout& operator=(VulkanDescriptorSetLayout&& other) noexcept = delete;
	~VulkanDescriptorSetLayout();
	std::shared_ptr<VulkanDescriptorSet> createDescriptorSet(
		std::shared_ptr<VulkanDescriptorPool> pool) const;
	const VkDescriptorSetLayoutBinding& getBinding(uint32_t binding) const;
	const VkDescriptorSetLayoutBinding& getBinding(const std::string& name) const;
	uint32_t queryBinding(const std::string& name) const;
	const std::vector<VkDescriptorSetLayoutBinding>& getBindings() const { return desc.bindings; }

private:
	VulkanDescriptorSetLayout(std::shared_ptr<const VulkanDevice> device,
							  const DescriptorSetLayoutDesc& desc,
							  VkDescriptorSetLayout layout,
							  std::unordered_map<std::string, uint32_t> nameToBinding);
	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	std::unordered_map<uint32_t, uint32_t> bindingToIndex;
	std::unordered_map<std::string, uint32_t> nameToBinding;
};

}  // namespace xd

#endif	// XD_RT_VULKANDESCRIPTORSETLAYOUT_H
