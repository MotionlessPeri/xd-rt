//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_VULKANSURFACE_H
#define XD_RT_VULKANSURFACE_H
#include <memory>
#include "VulkanPlatformSpecific.h"
#include "app/vulkan/Types.h"
#include "backend/vulkan/VulkanTypes.h"
namespace xd {
class VulkanSurface {
public:
	friend class VulkanDevice;

	VulkanSurface() = delete;
	VulkanSurface(const VulkanSurface& other) = delete;
	VulkanSurface(VulkanSurface&& other) noexcept = delete;
	VulkanSurface& operator=(const VulkanSurface& other) = delete;
	VulkanSurface& operator=(VulkanSurface&& other) noexcept = delete;
	~VulkanSurface();
	std::shared_ptr<VulkanSwapchain> createSwapchain(int width, int height) const;

private:
	VulkanSurface(std::weak_ptr<const VulkanInstance> instance,
				  std::weak_ptr<const VulkanPhysicalDevice> physicalDevice,
				  std::weak_ptr<const VulkanDevice> device,
				  VkSurfaceKHR surface);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
								int width,
								int height) const;
	std::weak_ptr<const VulkanInstance> instanceWeakRef;
	std::weak_ptr<const VulkanPhysicalDevice> physicalDeviceWeakRef;
	std::weak_ptr<const VulkanDevice> deviceWeakRef;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
};
}  // namespace xd
#endif	// XD_RT_VULKANSURFACE_H
