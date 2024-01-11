//
// Created by Frank on 2024/1/9.
//

#include "VulkanPhysicalDevice.h"
#include <cassert>
#include <stdexcept>
#include "VulkanDevice.h"
#include "VulkanMacros.h"
using namespace xd;

VulkanPhysicalDevice::VulkanPhysicalDevice(
	VkPhysicalDevice m_device,
	const std::weak_ptr<const VulkanInstance>& instanceWeakRef)
	: device(m_device), instanceWeakRef(instanceWeakRef)
{
	assert(m_device != VK_NULL_HANDLE);
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_device, &queueFamilyCount, nullptr);

	queueFamilyProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_device, &queueFamilyCount,
											 queueFamilyProperties.data());

	std::vector<VkExtensionProperties> supportedDeviceExtensions;
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extensionCount, nullptr);
	supportedDeviceExtensions.resize(extensionCount);
	vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extensionCount,
										 supportedDeviceExtensions.data());
	for (const auto& extension : supportedDeviceExtensions) {
		supportedExtensionNames.emplace(extension.extensionName);
	}
}

std::shared_ptr<VulkanDevice> VulkanPhysicalDevice::createLogicalDevice(
	std::vector<uint32_t> queueFamilyIndexes,
	std::vector<std::vector<float>> queuePriorities,
	VkPhysicalDeviceFeatures features,
	std::vector<const char*> deviceExtensions) const
{
	const auto queueFamilyCount = queueFamilyIndexes.size();
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(queueFamilyCount);
	for (auto i = 0ull; i < queueFamilyCount; ++i) {
		auto& queueCreateInfo = queueCreateInfos[i];
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilyIndexes[i];
		queueCreateInfo.queueCount = queuePriorities[i].size();
		queueCreateInfo.pQueuePriorities = queuePriorities[i].data();
	}

	for (const auto* extension : deviceExtensions) {
		if (supportedExtensionNames.count(extension) == 0) {
			throw std::runtime_error{""};
		}
	}

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = queueCreateInfos.size();
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &features;
	createInfo.enabledExtensionCount = deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	VkDevice logicalDevice;
	CHECK_VK_ERROR(vkCreateDevice(device, &createInfo, nullptr, &logicalDevice));
	return std::shared_ptr<VulkanDevice>(
		new VulkanDevice{logicalDevice, shared_from_this(), instanceWeakRef});
}

VkPhysicalDeviceProperties VulkanPhysicalDevice::getPhysicalDeviceProperties() const
{
	VkPhysicalDeviceProperties ret;
	vkGetPhysicalDeviceProperties(device, &ret);
	return ret;
}
VkPhysicalDeviceFeatures VulkanPhysicalDevice::getPhysicalDeviceFeatures() const
{
	VkPhysicalDeviceFeatures ret;
	vkGetPhysicalDeviceFeatures(device, &ret);
	return ret;
}

std::vector<VkSurfaceFormatKHR> VulkanPhysicalDevice::getPhysicalDeviceSurfaceFormats(
	VkSurfaceKHR surface) const
{
	uint32_t formatCount;
	std::vector<VkSurfaceFormatKHR> formats;
	CHECK_VK_ERROR(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr));
	if (formatCount != 0) {
		formats.resize(formatCount);
		CHECK_VK_ERROR(
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data()));
	}
	return formats;
}

std::vector<VkPresentModeKHR> VulkanPhysicalDevice::getPhysicalDeviceSurfacePresentModes(
	VkSurfaceKHR surface) const
{
	std::vector<VkPresentModeKHR> presentModes;
	uint32_t presentModeCount;
	CHECK_VK_ERROR(
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr));
	if (presentModeCount != 0) {
		presentModes.resize(presentModeCount);
		CHECK_VK_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
																 presentModes.data()));
	}
	return presentModes;
}

VkSurfaceCapabilitiesKHR VulkanPhysicalDevice::getPhysicalDeviceSurfaceCapabilities(
	VkSurfaceKHR surface) const
{
	VkSurfaceCapabilitiesKHR ret;
	CHECK_VK_ERROR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &ret));
	return ret;
}
