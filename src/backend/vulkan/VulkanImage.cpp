//
// Created by Frank on 2024/1/10.
//

#include "VulkanImage.h"
#include "VulkanDevice.h"
using namespace xd;
VulkanImage::VulkanImage(std::weak_ptr<const VulkanDevice> device,
						 VkImage image,
						 bool isSwapchainImage)
	: deviceWeakRef(device), image(image), isSwapchainImage(isSwapchainImage)
{
}
VulkanImage::~VulkanImage()
{
	if (!isSwapchainImage)
		deviceWeakRef.lock()->destroyImage(image);
}

std::shared_ptr<VulkanImageView> VulkanImage::createImageView(VkImageViewCreateInfo&& ci) const
{
	ci.image = image;
	return deviceWeakRef.lock()->createImageView(ci);
}
