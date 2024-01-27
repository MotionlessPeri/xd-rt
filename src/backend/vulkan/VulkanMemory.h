//
// Created by Frank on 2024/1/13.
//

#ifndef XD_RT_VULKANMEMORY_H
#define XD_RT_VULKANMEMORY_H
#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
namespace xd {

class VulkanMemory : public VulkanDeviceObject<VkMemoryAllocateInfo> {
public:
	friend class VulkanDevice;
	friend class VulkanBuffer;
	friend class VulkanImage;
	VulkanMemory() = delete;
	VulkanMemory(const VulkanMemory& other) = delete;
	VulkanMemory(VulkanMemory&& other) noexcept = delete;
	VulkanMemory& operator=(const VulkanMemory& other) = delete;
	VulkanMemory& operator=(VulkanMemory&& other) noexcept = delete;
	~VulkanMemory();
	void map(uint32_t offset, const void* ptr, uint32_t size) const;

private:
	VulkanMemory(std::shared_ptr<const VulkanDevice> device,
				 VkMemoryAllocateInfo desc,
				 VkDeviceMemory memory);
	VkDeviceMemory memory;
};

}  // namespace xd

#endif	// XD_RT_VULKANMEMORY_H
