//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_VULKANSURFACE_H
#define XD_RT_VULKANSURFACE_H
#include <memory>
#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {
class VulkanSurface : public VulkanDeviceObject<SurfaceCIType> {
public:
	friend class VulkanDevice;

	VulkanSurface() = delete;
	VulkanSurface(const VulkanSurface& other) = delete;
	VulkanSurface(VulkanSurface&& other) noexcept = delete;
	VulkanSurface& operator=(const VulkanSurface& other) = delete;
	VulkanSurface& operator=(VulkanSurface&& other) noexcept = delete;
	~VulkanSurface();
	std::shared_ptr<VulkanSwapchain> createSwapchain(int width,
													 int height,
													 VkImageUsageFlags imageUsages,
													 const VkSurfaceFormatKHR& desiredFormat) const;

	VulkanSurface(std::shared_ptr<const VulkanDevice> device,
				  VkWin32SurfaceCreateInfoKHR desc,
				  std::shared_ptr<const VulkanInstance> instance,
				  std::shared_ptr<const VulkanPhysicalDevice> physical_device,
				  VkSurfaceKHR surface);

private:
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
								int width,
								int height) const;
	std::shared_ptr<const VulkanInstance> instance;
	std::shared_ptr<const VulkanPhysicalDevice> physicalDevice;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
};
}  // namespace xd
#endif	// XD_RT_VULKANSURFACE_H
