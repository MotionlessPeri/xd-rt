//
// Created by Frank on 2024/1/9.
//

#ifndef XD_RT_VULKANPHYSICALDEVICE_H
#define XD_RT_VULKANPHYSICALDEVICE_H

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {

class VulkanPhysicalDevice : public std::enable_shared_from_this<VulkanPhysicalDevice> {
public:
	friend class VulkanInstance;
	VulkanPhysicalDevice() = delete;
	VulkanPhysicalDevice(const VulkanPhysicalDevice& other) = delete;
	VulkanPhysicalDevice(VulkanPhysicalDevice&& other) noexcept = delete;
	VulkanPhysicalDevice& operator=(const VulkanPhysicalDevice& other) = delete;
	VulkanPhysicalDevice& operator=(VulkanPhysicalDevice&& other) noexcept = delete;
	~VulkanPhysicalDevice() = default;
	template <typename CheckFuncType>
	int getSuitableQueueFamilyIndex(CheckFuncType func) const
	{
		for (int i = 0; i < queueFamilyProperties.size(); ++i) {
			const auto& properties = queueFamilyProperties[i];
			if (func(device, properties, i)) {
				return i;
			}
		}
		return -1;
	}
	// VkPhysicalDevice getHandle() const { return device; }
	std::shared_ptr<VulkanDevice> createLogicalDevice(
		std::vector<uint32_t> queueFamilyIndexes,
		std::vector<std::vector<float>> queuePriorities,
		VkPhysicalDeviceFeatures features,
		std::vector<const char*> deviceExtensions) const;

	VkPhysicalDeviceProperties getPhysicalDeviceProperties() const;
	VkPhysicalDeviceFeatures getPhysicalDeviceFeatures() const;
	std::vector<VkSurfaceFormatKHR> getPhysicalDeviceSurfaceFormats(VkSurfaceKHR surface) const;
	std::vector<VkPresentModeKHR> getPhysicalDeviceSurfacePresentModes(VkSurfaceKHR surface) const;
	VkSurfaceCapabilitiesKHR getPhysicalDeviceSurfaceCapabilities(VkSurfaceKHR surface) const;

private:
	explicit VulkanPhysicalDevice(VkPhysicalDevice m_device,
								  const std::weak_ptr<const VulkanInstance>& instanceWeakRef);
	VkPhysicalDevice device = VK_NULL_HANDLE;
	std::weak_ptr<const VulkanInstance> instanceWeakRef;
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	std::unordered_set<std::string> supportedExtensionNames;
};

}  // namespace xd

#endif	// XD_RT_VULKANPHYSICALDEVICE_H
