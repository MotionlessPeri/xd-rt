//
// Created by Frank on 2024/1/18.
//

#ifndef XD_RT_VULKANFENCE_H
#define XD_RT_VULKANFENCE_H
#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
namespace xd {

class VulkanFence : public VulkanDeviceObject<VkFenceCreateInfo> {
public:
	friend class VulkanDevice;
	friend class VulkanSwapchain;
	friend class VulkanQueue;
	VulkanFence() = delete;
	VulkanFence(const VulkanFence& other) = delete;
	VulkanFence(VulkanFence&& other) noexcept = delete;
	VulkanFence& operator=(const VulkanFence& other) = delete;
	VulkanFence& operator=(VulkanFence&& other) noexcept = delete;
	~VulkanFence();
	void wait() const;
	void reset() const;

private:
	VulkanFence(std::shared_ptr<const VulkanDevice> device,
				const VkFenceCreateInfo& desc,
				VkFence fence);

	VkFence fence = VK_NULL_HANDLE;
};

}  // namespace xd

#endif	// XD_RT_VULKANFENCE_H
