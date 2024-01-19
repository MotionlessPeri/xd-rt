//
// Created by Frank on 2024/1/9.
//

#ifndef XD_RT_VULKANPHYSICALDEVICE_H
#define XD_RT_VULKANPHYSICALDEVICE_H

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>
#include "VulkanDescs.h"
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
	std::shared_ptr<VulkanDevice> createLogicalDevice(const DeviceDesc& desc) const;

	VkPhysicalDeviceProperties getPhysicalDeviceProperties() const;
	VkPhysicalDeviceProperties2 getPhysicalDeviceProperties2() const;
	VkPhysicalDeviceFeatures getPhysicalDeviceFeatures() const;
	std::vector<VkSurfaceFormatKHR> getPhysicalDeviceSurfaceFormats(VkSurfaceKHR surface) const;
	std::vector<VkPresentModeKHR> getPhysicalDeviceSurfacePresentModes(VkSurfaceKHR surface) const;
	VkSurfaceCapabilitiesKHR getPhysicalDeviceSurfaceCapabilities(VkSurfaceKHR surface) const;
	uint32_t getProperMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
	const VkPhysicalDeviceMemoryProperties& getMemoryProperties() const { return memProperties; }

private:
	explicit VulkanPhysicalDevice(VkPhysicalDevice m_device,
								  std::shared_ptr<const VulkanInstance> instanceWeakRef);
	VkPhysicalDevice device = VK_NULL_HANDLE;
	std::shared_ptr<const VulkanInstance> instance;
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	std::unordered_set<std::string> supportedExtensionNames;
	VkPhysicalDeviceMemoryProperties memProperties;
};

}  // namespace xd

#endif	// XD_RT_VULKANPHYSICALDEVICE_H
