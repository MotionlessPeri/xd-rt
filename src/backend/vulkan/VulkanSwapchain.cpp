//
// Created by Frank on 2024/1/10.
//

#include "VulkanSwapchain.h"

#include "backend/vulkan/VulkanDevice.h"
#include "backend/vulkan/VulkanImage.h"

namespace xd {

VulkanSwapchain::VulkanSwapchain(std::weak_ptr<const VulkanDevice> device,
								 VkSwapchainKHR swapchain,
								 VkExtent2D extent,
								 VkSurfaceFormatKHR format)
	: deviceWeakRef(std::move(device)), swapchain(swapchain), extent(extent), format(format)
{
	retrieveSwapchainImages();
}

VulkanSwapchain::~VulkanSwapchain()
{
	deviceWeakRef.lock()->destroySwapchain(swapchain);
}

void VulkanSwapchain::retrieveSwapchainImages()
{
	swapchainImages = deviceWeakRef.lock()->getSwapchainImages(swapchain);
}
}  // namespace xd