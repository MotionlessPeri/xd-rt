//
// Created by Frank on 2024/1/9.
//

#ifndef XD_RT_VULKANDEVICE_H
#define XD_RT_VULKANDEVICE_H
#include <unordered_map>
#include "MathUtil.h"
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {

class VulkanDevice : public std::enable_shared_from_this<VulkanDevice> {
public:
	friend class VulkanPhysicalDevice;
	VulkanDevice() = delete;
	VulkanDevice(const VulkanDevice& other) = delete;
	VulkanDevice(VulkanDevice&& other) noexcept = delete;
	VulkanDevice& operator=(const VulkanDevice& other) = delete;
	VulkanDevice& operator=(VulkanDevice&& other) noexcept = delete;
	~VulkanDevice();
	std::shared_ptr<VulkanQueue> getQueue(int queueFamilyIndex, int queueIndex);
	std::shared_ptr<VulkanQueue> getQueue(const std::pair<int, int>& key);
	// VkDevice getHandle() const { return device; }

	std::shared_ptr<VulkanSurface> createSurface(const SurfaceCIType& ci) const;
	std::shared_ptr<VulkanSurface> createSurface(VkSurfaceKHR surfaceHandle) const;
	void destroySurface(VkSurfaceKHR surface) const;
	std::shared_ptr<VulkanSwapchain> createSwapchain(const VkSwapchainCreateInfoKHR& ci,
													 VkExtent2D extent,
													 VkSurfaceFormatKHR format) const;
	void destroySwapchain(VkSwapchainKHR swapchainHandle) const;
	std::vector<std::shared_ptr<VulkanImage>> getSwapchainImages(VkSwapchainKHR swapchain) const;
	void destroyImage(VkImage image) const;
	std::shared_ptr<VulkanImageView> createImageView(const VkImageViewCreateInfo& ci) const;
	void destroyImageView(VkImageView imageView) const;

private:
	VulkanDevice(VkDevice device,
				 std::weak_ptr<const VulkanPhysicalDevice> physical_device_weak_ref,
				 std::weak_ptr<const VulkanInstance> instance_weak_ref);
	VkDevice device;
	std::weak_ptr<const VulkanPhysicalDevice> physicalDeviceWeakRef;
	std::weak_ptr<const VulkanInstance> instanceWeakRef;
	std::unordered_map<std::pair<int, int>, std::shared_ptr<VulkanQueue>, PairHasher<int, int>>
		queues;
};

}  // namespace xd

#endif	// XD_RT_VULKANDEVICE_H
