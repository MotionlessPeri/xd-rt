//
// Created by Frank on 2024/1/9.
//

#include "VulkanPhysicalDevice.h"
#include <cassert>
#include <stdexcept>
#include "VulkanDevice.h"
#include "VulkanMacros.h"
using namespace xd;

VulkanPhysicalDevice::VulkanPhysicalDevice(VkPhysicalDevice m_device,
										   std::shared_ptr<const VulkanInstance> instance)
	: device(m_device), instance(std::move(instance))
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

	vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
}

std::shared_ptr<VulkanDevice> VulkanPhysicalDevice::createLogicalDevice(
	const DeviceDesc& desc) const
{
	VkDevice logicalDevice;
	CHECK_VK_ERROR(vkCreateDevice(device, &desc.ci, nullptr, &logicalDevice));
	return std::shared_ptr<VulkanDevice>{
		new VulkanDevice{logicalDevice, desc, shared_from_this(), instance}};
}

VkPhysicalDeviceProperties VulkanPhysicalDevice::getPhysicalDeviceProperties() const
{
	VkPhysicalDeviceProperties ret;
	vkGetPhysicalDeviceProperties(device, &ret);
	return ret;
}

VkPhysicalDeviceProperties2 VulkanPhysicalDevice::getPhysicalDeviceProperties2() const
{
	VkPhysicalDeviceDepthStencilResolveProperties depthStencilProps;
	depthStencilProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES;
	depthStencilProps.pNext = nullptr;
	VkPhysicalDeviceProperties2 ret;
	ret.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	ret.pNext = &depthStencilProps;
	vkGetPhysicalDeviceProperties2(device, &ret);
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

uint32_t VulkanPhysicalDevice::getProperMemoryTypeIndex(uint32_t typeFilter,
														VkMemoryPropertyFlags properties) const
{
	for (const auto i : std::views::iota(0u, memProperties.memoryTypeCount)) {
		if ((typeFilter & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	throw std::runtime_error{"No suitable memory type!\n"};
}
