//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_VULKANSWAPCHAIN_H
#define XD_RT_VULKANSWAPCHAIN_H
#include <memory>
#include <vector>

#include "VulkanPlatformSpecific.h"
#include "backend/vulkan/VulkanTypes.h"
namespace xd {

class VulkanSwapchain {
public:
	friend class VulkanDevice;
	VulkanSwapchain(std::weak_ptr<const VulkanDevice> device,
					VkSwapchainKHR swapchain,
					VkExtent2D extent,
					VkSurfaceFormatKHR format);
	~VulkanSwapchain();

private:
	void retrieveSwapchainImages();
	std::weak_ptr<const VulkanDevice> deviceWeakRef;
	VkSwapchainKHR swapchain;
	VkExtent2D extent;
	VkSurfaceFormatKHR format;
	std::vector<std::shared_ptr<VulkanImage>> swapchainImages;
};

}  // namespace xd

#endif	// XD_RT_VULKANSWAPCHAIN_H
