//
// Created by Frank on 2024/1/10.
//
#include "VulkanSurface.h"

#include <bit>
#include <stdexcept>
#include "VulkanSwapchain.h"
#include "backend/vulkan/VulkanDevice.h"
#include "backend/vulkan/VulkanInstance.h"
#include "backend/vulkan/VulkanMacros.h"
using namespace xd;
VulkanSurface::VulkanSurface(std::weak_ptr<const VulkanInstance> instance,
							 std::weak_ptr<const VulkanPhysicalDevice> physicalDevice,
							 std::weak_ptr<const VulkanDevice> device,
							 VkSurfaceKHR surface)
	: instanceWeakRef(std::move(instance)),
	  physicalDeviceWeakRef(std::move(physicalDevice)),
	  deviceWeakRef(std::move(device)),
	  surface(surface)
{
}

VulkanSurface::~VulkanSurface()
{
	deviceWeakRef.lock()->destroySurface(surface);
}

std::shared_ptr<VulkanSwapchain> VulkanSurface::createSwapchain(int width, int height) const
{
	const auto physicalDevice = physicalDeviceWeakRef.lock();
	const auto device = deviceWeakRef.lock();
	if (!device)
		throw std::runtime_error{""};

	std::vector<VkSurfaceFormatKHR> formats =
		physicalDevice->getPhysicalDeviceSurfaceFormats(surface);
	if (formats.empty())
		throw std::runtime_error{""};
	VkSurfaceFormatKHR chosenFormat = formats.front();
	VkSurfaceFormatKHR desiredFormat{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
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
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = static_cast<VkCompositeAlphaFlagBitsKHR>(
		1 << std::countr_zero(capabilities.supportedCompositeAlpha));
	return deviceWeakRef.lock()->createSwapchain(createInfo, extent, chosenFormat);
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
