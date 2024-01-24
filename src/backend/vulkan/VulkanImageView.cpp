//
// Created by Frank on 2024/1/10.
//

#include "VulkanImageView.h"

#include "VulkanDevice.h"

using namespace xd;

VulkanImageView::~VulkanImageView()
{
	device->destroyImageView(imageView);
}

VulkanImageView::VulkanImageView(std::shared_ptr<const VulkanDevice> device,
								 const VkImageViewCreateInfo& desc,
								 std::shared_ptr<const VulkanImage> image_ref,
								 VkImageView image_view)
	: VulkanDeviceObject(std::move(device), desc),
	  imageRef(std::move(image_ref)),
	  imageView(image_view)
{
}
