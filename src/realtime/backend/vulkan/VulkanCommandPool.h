//
// Created by Frank on 2024/1/14.
//

#ifndef XD_RT_VULKANCOMMANDPOOL_H
#define XD_RT_VULKANCOMMANDPOOL_H
#include <vector>

#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
namespace xd {

class VulkanCommandPool : public VulkanDeviceObject<VkCommandPoolCreateInfo>,
						  public std::enable_shared_from_this<VulkanCommandPool> {
public:
	friend class VulkanDevice;
	friend class VulkanCommandBuffer;
	friend class ImguiAppBase;	// TODO: find a better way to encapsulate imgui, maybe in the lib?
	VulkanCommandPool() = delete;
	VulkanCommandPool(const VulkanCommandPool& other) = delete;
	VulkanCommandPool(VulkanCommandPool&& other) noexcept = delete;
	VulkanCommandPool& operator=(const VulkanCommandPool& other) = delete;
	VulkanCommandPool& operator=(VulkanCommandPool&& other) noexcept = delete;
	~VulkanCommandPool();
	std::vector<std::shared_ptr<VulkanCommandBuffer>> allocateCommandBuffers(
		VkCommandBufferAllocateInfo&& ai) const;
	void freeCommandBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const;
	void freeCommandBuffers(
		const std::vector<std::shared_ptr<VulkanCommandBuffer>>& cmdBuffers) const;
	std::shared_ptr<VulkanQueue> getQueue() const { return queue; }

private:
	VulkanCommandPool(std::shared_ptr<const VulkanDevice> device,
					  const VkCommandPoolCreateInfo& desc,
					  std::shared_ptr<VulkanQueue> queue,
					  VkCommandPool pool);

	std::shared_ptr<VulkanQueue> queue = nullptr;
	VkCommandPool pool = VK_NULL_HANDLE;
};

}  // namespace xd

#endif	// XD_RT_VULKANCOMMANDPOOL_H
