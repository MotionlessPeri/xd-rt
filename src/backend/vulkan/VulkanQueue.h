//
// Created by Frank on 2024/1/9.
//

#ifndef XD_RT_VULKANQUEUE_H
#define XD_RT_VULKANQUEUE_H
#include <vector>
#include "VulkanDescs.h"
#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
namespace xd {

class VulkanQueue : public VulkanDeviceObject<QueueDesc> {
public:
	friend class VulkanDevice;
	friend class ImguiAppBase;	// TODO: find a better way to encapsulate imgui, maybe in the lib?
	VulkanQueue() = delete;
	VulkanQueue(const VulkanQueue& other) = delete;
	VulkanQueue(VulkanQueue&& other) noexcept = delete;
	VulkanQueue& operator=(const VulkanQueue& other) = delete;
	VulkanQueue& operator=(VulkanQueue&& other) noexcept = delete;
	~VulkanQueue() = default;
	void submitAndWait(const std::vector<VkSubmitInfo>& infos,
					   std::shared_ptr<VulkanFence> signalingFence) const;
	void submit(const std::vector<VkSubmitInfo>& infos,
				std::shared_ptr<VulkanFence> signalingFence) const;
	void waitIdle() const;
	uint32_t getQueueFamilyIndex() const { return desc.deviceQueueDesc->ci.queueFamilyIndex; }
	uint32_t getQueueIndex() const { return desc.index; }
	void present(const QueuePresentInfoContainer& desc) const;

private:
	VulkanQueue(std::shared_ptr<const VulkanDevice> device, const QueueDesc& desc, VkQueue queue);
	VkQueue queue;
};

}  // namespace xd

#endif	// XD_RT_VULKANQUEUE_H
