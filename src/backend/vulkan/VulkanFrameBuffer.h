//
// Created by Frank on 2024/1/17.
//

#ifndef XD_RT_VULKANFRAMEBUFFER_H
#define XD_RT_VULKANFRAMEBUFFER_H
#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
namespace xd {

class VulkanFrameBuffer : public VulkanDeviceObject<VkFramebufferCreateInfo> {
public:
	friend class VulkanDevice;
	friend class BasicGraphicApp;  // TODO: remove it ASAP when FrameGraph is built
	VulkanFrameBuffer() = delete;
	VulkanFrameBuffer(const VulkanFrameBuffer& other) = delete;
	VulkanFrameBuffer(VulkanFrameBuffer&& other) noexcept = delete;
	VulkanFrameBuffer& operator=(const VulkanFrameBuffer& other) = delete;
	VulkanFrameBuffer& operator=(VulkanFrameBuffer&& other) noexcept = delete;
	~VulkanFrameBuffer();

private:
	VulkanFrameBuffer(std::shared_ptr<const VulkanDevice> device,
					  const VkFramebufferCreateInfo& desc,
					  VkFramebuffer frame_buffer);
	VkFramebuffer frameBuffer = VK_NULL_HANDLE;
};

}  // namespace xd

#endif	// XD_RT_VULKANFRAMEBUFFER_H
