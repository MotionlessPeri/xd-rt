//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_VULKANIMAGE_H
#define XD_RT_VULKANIMAGE_H

#include <memory>

#include "VulkanDeviceObject.h"
#include "VulkanMemory.h"
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {

class VulkanImage : public VulkanDeviceObject<VkImageCreateInfo> {
public:
	friend class VulkanDevice;
	VulkanImage() = delete;
	VulkanImage(const VulkanImage& other) = delete;
	VulkanImage(VulkanImage&& other) noexcept = delete;
	VulkanImage& operator=(const VulkanImage& other) = delete;
	VulkanImage& operator=(VulkanImage&& other) noexcept = delete;
	~VulkanImage();
	std::shared_ptr<VulkanImageView> createImageView(VkImageViewCreateInfo&& ci) const;
	void setData(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
				 uint32_t offset,
				 void* ptr,
				 uint32_t size) const;
	void transitState(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
					  VkPipelineStageFlags srcStageMask,
					  VkPipelineStageFlags dstStageMask,
					  VkImageMemoryBarrier&& imageBarrier) const;

private:
	VulkanImage(std::shared_ptr<const VulkanDevice> device, VkImage image);
	VulkanImage(std::shared_ptr<const VulkanDevice> device,
				VkImageCreateInfo desc,
				VkImage image,
				VkMemoryPropertyFlags properties);
	VkImage image = VK_NULL_HANDLE;
	std::unique_ptr<VulkanMemory> memory;
	bool isSwapchainImage = false;
};

}  // namespace xd

#endif	// XD_RT_VULKANIMAGE_H
