//
// Created by Frank on 2024/1/10.
//

#include "VulkanImageView.h"

#include "VulkanDevice.h"

using namespace xd;
VulkanImageView::~VulkanImageView()
{
	deviceWeakRef.lock()->destroyImageView(imageView);
}

VulkanImageView::VulkanImageView(std::weak_ptr<const VulkanDevice> device, VkImageView image_view)
	: deviceWeakRef(device), imageView(image_view)
{
}
