//
// Created by Frank on 2024/1/10.
//
#include "VulkanSurface.h"

#include <bit>
#include <stdexcept>
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanSwapchain.h"

using namespace xd;
VulkanSurface::VulkanSurface(std::shared_ptr<const VulkanDevice> device,
							 VkWin32SurfaceCreateInfoKHR desc,
							 std::shared_ptr<const VulkanInstance> instance,
							 std::shared_ptr<const VulkanPhysicalDevice> physical_device,
							 VkSurfaceKHR surface)
	: VulkanDeviceObject(std::move(device), std::move(desc)),
	  instance(std::move(instance)),
	  physicalDevice(std::move(physical_device)),
	  surface(surface)
{
}

VulkanSurface::~VulkanSurface()
{
	device->destroySurface(surface);
}

std::shared_ptr<VulkanSwapchain> VulkanSurface::createSwapchain(
	int width,
	int height,
	VkImageUsageFlags imageUsages,
	const VkSurfaceFormatKHR& desiredFormat) const
{
	if (!device)
		throw std::runtime_error{""};

	std::vector<VkSurfaceFormatKHR> formats =
		physicalDevice->getPhysicalDeviceSurfaceFormats(surface);
	if (formats.empty())
		throw std::runtime_error{""};
	VkSurfaceFormatKHR chosenFormat = formats.front();
	// VkSurfaceFormatKHR desiredFormat{VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
	for (const auto& formatCand : formats) {
		if (formatCand.format == desiredFormat.format &&
			formatCand.colorSpace == desiredFormat.colorSpace) {
			chosenFormat = desiredFormat;
			break;
		}
	}

	std::vector<VkPresentModeKHR> presentModes =
		physicalDevice->getPhysicalDeviceSurfacePresentModes(surface);
	if (presentModes.empty())
		throw std::runtime_error{""};
	VkPresentModeKHR chosenPresentMode = presentModes.front();
	VkPresentModeKHR desiredPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (const auto modeCand : presentModes) {
		if (modeCand == desiredPresentMode) {
			chosenPresentMode = desiredPresentMode;
			break;
		}
	}

	VkSurfaceCapabilitiesKHR capabilities =
		physicalDevice->getPhysicalDeviceSurfaceCapabilities(surface);
	VkExtent2D extent = chooseSwapExtent(capabilities, width, height);

	uint32_t imageCount = capabilities.minImageCount + 1;

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.presentMode = chosenPresentMode;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = chosenFormat.format;
	createInfo.imageColorSpace = chosenFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = imageUsages;
	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = static_cast<VkCompositeAlphaFlagBitsKHR>(
		1 << std::countr_zero(capabilities.supportedCompositeAlpha));
	return device->createSwapchain(std::move(createInfo), extent, chosenFormat);
}

VkExtent2D VulkanSurface::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
										   int width,
										   int height) const
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
										capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
										 capabilities.maxImageExtent.height);
		return actualExtent;
	}
}
