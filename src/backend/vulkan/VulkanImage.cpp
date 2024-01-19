//
// Created by Frank on 2024/1/10.
//

#include "VulkanImage.h"
#include "VulkanDevice.h"
using namespace xd;
VulkanImage::VulkanImage(std::shared_ptr<const VulkanDevice> _device, VkImage image)
	: VulkanDeviceObject(std::move(_device)), image(image), isSwapchainImage(true)
{
}

VulkanImage::VulkanImage(std::shared_ptr<const VulkanDevice> _device,
						 VkImageCreateInfo _desc,
						 VkImage image)
	: VulkanDeviceObject(std::move(_device), std::move(_desc)),
	  image(image),
	  isSwapchainImage(false)
{
}

VulkanImage::~VulkanImage()
{
	if (!isSwapchainImage)
		device->destroyImage(image);
}

std::shared_ptr<VulkanImageView> VulkanImage::createImageView(VkImageViewCreateInfo&& ci) const
{
	ci.image = image;
	return device->createImageView(ci);
}
