//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_VULKANSWAPCHAIN_H
#define XD_RT_VULKANSWAPCHAIN_H
#include <memory>
#include <vector>
#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {

class VulkanSwapchain : public VulkanDeviceObject<VkSwapchainCreateInfoKHR> {
public:
	friend class VulkanDevice;
	friend class VulkanQueue;
	~VulkanSwapchain();
	struct SwapchainImage {
		std::shared_ptr<VulkanImage> image;
		std::shared_ptr<VulkanImageView> view;
	};
	VkExtent2D getExtent() const { return extent; }
	VkSurfaceFormatKHR getFormat() const { return format; }
	uint32_t getSwapchainImageCount() const { return swapchainImages.size(); }
	const std::vector<SwapchainImage>& getSwapchainImages() const { return swapchainImages; }
	uint32_t acquireNextImage(std::shared_ptr<VulkanSemaphore> semaphore,
							  std::shared_ptr<VulkanFence> fence) const;

private:
	VulkanSwapchain(std::shared_ptr<const VulkanDevice> device,
					const VkSwapchainCreateInfoKHR& desc,
					VkSwapchainKHR swapchain,
					VkExtent2D extent,
					VkSurfaceFormatKHR format);
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkExtent2D extent{};
	VkSurfaceFormatKHR format{};
	std::vector<SwapchainImage> swapchainImages{};
};

}  // namespace xd

#endif	// XD_RT_VULKANSWAPCHAIN_H
