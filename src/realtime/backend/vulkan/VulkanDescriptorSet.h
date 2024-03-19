//
// Created by Frank on 2024/1/15.
//

#ifndef XD_RT_VULKANDESCRIPTORSET_H
#define XD_RT_VULKANDESCRIPTORSET_H

#include <unordered_map>

#include "VulkanDescriptorSetLayout.h"
#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
namespace xd {
class VulkanDescriptorSet : public VulkanDeviceObject<VkDescriptorSetAllocateInfo> {
public:
	friend class VulkanDevice;
	friend class VulkanPipelineLayout;
	VulkanDescriptorSet() = delete;
	VulkanDescriptorSet(const VulkanDescriptorSet& other) = delete;
	VulkanDescriptorSet(VulkanDescriptorSet&& other) noexcept = delete;
	VulkanDescriptorSet& operator=(const VulkanDescriptorSet& other) = delete;
	VulkanDescriptorSet& operator=(VulkanDescriptorSet&& other) noexcept = delete;
	void bindResource(uint32_t binding, const VkDescriptorBufferInfo& bufferInfo);
	void bindResource(uint32_t binding, const VkDescriptorImageInfo& imageInfo);
	void bindResource(const std::string& name, const VkDescriptorBufferInfo& bufferInfo);
	void bindResource(const std::string& name, const VkDescriptorImageInfo& imageInfo);
	void updateDescriptors();

private:
	VulkanDescriptorSet(std::shared_ptr<const VulkanDevice> device,
						const VkDescriptorSetAllocateInfo& desc,
						std::shared_ptr<const VulkanDescriptorSetLayout> layout_ref,
						std::shared_ptr<const VulkanDescriptorPool> pool_ref,
						VkDescriptorSet set);

	template <typename DescInfoType>
	void addWriteDescriptorSet(std::vector<VkWriteDescriptorSet>& writes,
							   std::unordered_map<uint32_t, DescInfoType>& map)
	{
		for (const auto& it : map) {
			const auto& layoutBinding = layoutRef->getBinding(it.first);
			VkWriteDescriptorSet writeDescriptorSet{};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = set;
			writeDescriptorSet.descriptorType = layoutBinding.descriptorType;
			writeDescriptorSet.dstBinding = it.first;
			if constexpr (std::is_same_v<std::remove_cv_t<DescInfoType>, VkDescriptorBufferInfo>) {
				writeDescriptorSet.pBufferInfo = &it.second;
			}
			else if constexpr (std::is_same_v<std::remove_cv_t<DescInfoType>,
											  VkDescriptorImageInfo>) {
				writeDescriptorSet.pImageInfo = &it.second;
			}
			else {
				[]<bool flag = false>()
				{
					static_assert(flag, "no match");
				}
				();
			}
			writeDescriptorSet.descriptorCount = 1;
			writes.emplace_back(writeDescriptorSet);
		}
	}

	std::shared_ptr<const VulkanDescriptorSetLayout> layoutRef = nullptr;
	std::shared_ptr<const VulkanDescriptorPool> poolRef = nullptr;
	std::unordered_map<uint32_t, VkDescriptorBufferInfo> boundBuffers;
	std::unordered_map<uint32_t, VkDescriptorImageInfo> boundImages;
	VkDescriptorSet set = VK_NULL_HANDLE;
};

}  // namespace xd

#endif	// XD_RT_VULKANDESCRIPTORSET_H
