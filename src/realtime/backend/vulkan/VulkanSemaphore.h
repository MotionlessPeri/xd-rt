//
// Created by Frank on 2024/1/18.
//

#ifndef XD_RT_VULKANSEMAPHORE_H
#define XD_RT_VULKANSEMAPHORE_H
#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
namespace xd {

class VulkanSemaphore : public VulkanDeviceObject<VkSemaphoreCreateInfo> {
public:
	friend class VulkanDevice;
	friend class VulkanSwapchain;
	friend class VulkanQueue;
	friend class VulkanCommandBuffer;
	VulkanSemaphore() = delete;
	VulkanSemaphore(const VulkanSemaphore& other) = delete;
	VulkanSemaphore(VulkanSemaphore&& other) noexcept = delete;
	VulkanSemaphore& operator=(const VulkanSemaphore& other) = delete;
	VulkanSemaphore& operator=(VulkanSemaphore&& other) noexcept = delete;
	~VulkanSemaphore();

private:
	VulkanSemaphore(std::shared_ptr<const VulkanDevice> device,
					const VkSemaphoreCreateInfo& desc,
					VkSemaphore semaphore);

	VkSemaphore semaphore = VK_NULL_HANDLE;
};

}  // namespace xd

#endif	// XD_RT_VULKANSEMAPHORE_H
