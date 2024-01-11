//
// Created by Frank on 2024/1/9.
//

#ifndef XD_RT_VULKANQUEUE_H
#define XD_RT_VULKANQUEUE_H
#include "VulkanPlatformSpecific.h"
namespace xd {

class VulkanQueue {
public:
	explicit VulkanQueue(VkQueue queue) : queue(queue) {}
	VulkanQueue() = delete;
	VulkanQueue(const VulkanQueue& other) = delete;
	VulkanQueue(VulkanQueue&& other) noexcept = delete;
	VulkanQueue& operator=(const VulkanQueue& other) = delete;
	VulkanQueue& operator=(VulkanQueue&& other) noexcept = delete;
	~VulkanQueue() = default;

private:
	VkQueue queue;
};

}  // namespace xd

#endif	// XD_RT_VULKANQUEUE_H
