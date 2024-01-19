//
// Created by Frank on 2024/1/10.
//

#include "VulkanImageView.h"

#include "VulkanDevice.h"

using namespace xd;

VulkanImageView::VulkanImageView(std::shared_ptr<const VulkanDevice> device,
								 VkImageViewCreateInfo desc,
								 VkImageView image_view)
	: VulkanDeviceObject<VkImageViewCreateInfo>(std::move(device), std::move(desc)),
	  imageView(image_view)
{
}

VulkanImageView::~VulkanImageView()
{
	device->destroyImageView(imageView);
}
