//
// Created by Frank on 2024/1/13.
//

#ifndef XD_RT_VULKANBUFFER_H
#define XD_RT_VULKANBUFFER_H
#include "VulkanDeviceObject.h"
#include "VulkanMemory.h"
#include "VulkanPlatformSpecific.h"
namespace xd {
class VulkanBuffer : public VulkanDeviceObject<VkBufferCreateInfo> {
public:
	friend class VulkanDevice;
	friend class TriangleMeshVk;
	VulkanBuffer() = delete;
	VulkanBuffer(const VulkanBuffer& other) = delete;
	VulkanBuffer(VulkanBuffer&& other) noexcept = delete;
	VulkanBuffer& operator=(const VulkanBuffer& other) = delete;
	VulkanBuffer& operator=(VulkanBuffer&& other) noexcept = delete;
	~VulkanBuffer();
	void setData(uint32_t offset, void* ptr, uint32_t size) const;
	VkDescriptorBufferInfo getBindingInfo(VkDeviceSize offset = 0,
										  VkDeviceSize range = VK_WHOLE_SIZE) const
	{
		VkDescriptorBufferInfo ret;
		ret.buffer = buffer;
		ret.offset = 0;
		ret.range = VK_WHOLE_SIZE;
		return ret;
	}

private:
	VulkanBuffer(std::shared_ptr<const VulkanDevice> _device,
				 VkBufferCreateInfo _desc,
				 VkBuffer buffer,
				 VkMemoryPropertyFlags properties);
	VkBuffer buffer = VK_NULL_HANDLE;
	std::unique_ptr<VulkanMemory> memory;
};

}  // namespace xd

#endif	// XD_RT_VULKANBUFFER_H
