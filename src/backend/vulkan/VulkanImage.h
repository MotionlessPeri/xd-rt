//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_VULKANIMAGE_H
#define XD_RT_VULKANIMAGE_H

#include <memory>

#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {

class VulkanImage {
public:
	friend class VulkanDevice;
	VulkanImage() = delete;
	VulkanImage(const VulkanImage& other) = delete;
	VulkanImage(VulkanImage&& other) noexcept = delete;
	VulkanImage& operator=(const VulkanImage& other) = delete;
	VulkanImage& operator=(VulkanImage&& other) noexcept = delete;
	~VulkanImage();
	std::shared_ptr<VulkanImageView> createImageView(VkImageViewCreateInfo&& ci) const;

private:
	VulkanImage(std::weak_ptr<const VulkanDevice> device, VkImage image, bool isSwapchainImage);
	std::weak_ptr<const VulkanDevice> deviceWeakRef;
	VkImage image = VK_NULL_HANDLE;
	bool isSwapchainImage = false;
};

}  // namespace xd

#endif	// XD_RT_VULKANIMAGE_H
