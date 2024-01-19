//
// Created by Frank on 2024/1/10.
//

#include "VulkanSwapchain.h"

#include "VulkanDevice.h"
#include "VulkanFence.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanSemaphore.h"

namespace xd {
uint32_t VulkanSwapchain::acquireNextImage(std::shared_ptr<VulkanSemaphore> semaphore,
										   std::shared_ptr<VulkanFence> fence) const
{
	const VkSemaphore semaphoreHandle = semaphore ? semaphore->semaphore : VK_NULL_HANDLE;
	const VkFence fenceHandle = fence ? fence->fence : VK_NULL_HANDLE;
	return device->acquireNextImage(swapchain, semaphoreHandle, fenceHandle);
}

VulkanSwapchain::VulkanSwapchain(std::shared_ptr<const VulkanDevice> _device,
								 const VkSwapchainCreateInfoKHR& _desc,
								 VkSwapchainKHR _swapchain,
								 VkExtent2D _extent,
								 VkSurfaceFormatKHR _format)
	: VulkanDeviceObject(std::move(_device), _desc),
	  swapchain(_swapchain),
	  extent(std::move(_extent)),
	  format(std::move(_format))
{
	const auto images = device->getSwapchainImages(swapchain);
	swapchainImages.resize(images.size());
	std::ranges::transform(images, swapchainImages.begin(), [&](const auto& image) {
		SwapchainImage ret;
		ret.image = image;
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format.format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		ret.view = image->createImageView(std::move(createInfo));
		return ret;
	});
}

VulkanSwapchain::~VulkanSwapchain()
{
	device->destroySwapchain(swapchain);
}
}  // namespace xd