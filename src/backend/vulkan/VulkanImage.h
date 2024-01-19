//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_VULKANIMAGE_H
#define XD_RT_VULKANIMAGE_H

#include <memory>

#include "VulkanDeviceObject.h"
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

private:
	VulkanImage(std::shared_ptr<const VulkanDevice> device, VkImage image);
	VulkanImage(std::shared_ptr<const VulkanDevice> device, VkImageCreateInfo desc, VkImage image);
	VkImage image = VK_NULL_HANDLE;
	bool isSwapchainImage = false;
};

}  // namespace xd

#endif	// XD_RT_VULKANIMAGE_H
