//
// Created by Frank on 2024/1/9.
//

#include "VulkanDevice.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanInstance.h"
#include "VulkanMacros.h"
#include "VulkanQueue.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"
using namespace xd;
VulkanDevice::VulkanDevice(VkDevice device,
						   std::weak_ptr<const VulkanPhysicalDevice> physical_device_weak_ref,
						   std::weak_ptr<const VulkanInstance> instance_weak_ref)
	: device(device),
	  physicalDeviceWeakRef(std::move(physical_device_weak_ref)),
	  instanceWeakRef(std::move(instance_weak_ref))
{
}

VulkanDevice::~VulkanDevice()
{
	vkDestroyDevice(device, nullptr);
}

std::shared_ptr<VulkanQueue> VulkanDevice::getQueue(int queueFamilyIndex, int queueIndex)
{
	return getQueue({queueFamilyIndex, queueIndex});
}

std::shared_ptr<VulkanQueue> VulkanDevice::getQueue(const std::pair<int, int>& key)
{
	const auto it = queues.find(key);
	if (it != queues.end())
		return it->second;
	VkQueue queue;
	vkGetDeviceQueue(device, key.first, key.second, &queue);
	auto res = std::shared_ptr<VulkanQueue>{new VulkanQueue{queue}};
	queues[key] = res;
	return res;
}

std::shared_ptr<VulkanSurface> VulkanDevice::createSurface(const SurfaceCIType& ci) const
{
	VkSurfaceKHR surfaceHandle;
	const auto instance = instanceWeakRef.lock();
	if (!instance)
		assert(false);
	CHECK_VK_ERROR(vkCreateWin32SurfaceKHR(instance->instance, &ci, nullptr, &surfaceHandle));
	return createSurface(surfaceHandle);
}

std::shared_ptr<VulkanSurface> VulkanDevice::createSurface(VkSurfaceKHR surfaceHandle) const
{
	return std::shared_ptr<VulkanSurface>{new VulkanSurface{instanceWeakRef, physicalDeviceWeakRef,
															shared_from_this(), surfaceHandle}};
}

void VulkanDevice::destroySurface(VkSurfaceKHR surface) const
{
	vkDestroySurfaceKHR(instanceWeakRef.lock()->instance, surface, nullptr);
}

std::shared_ptr<VulkanSwapchain> VulkanDevice::createSwapchain(const VkSwapchainCreateInfoKHR& ci,
															   VkExtent2D extent,
															   VkSurfaceFormatKHR format) const
{
	VkSwapchainKHR swapchainHandle;
	CHECK_VK_ERROR(vkCreateSwapchainKHR(device, &ci, nullptr, &swapchainHandle));
	return std::shared_ptr<VulkanSwapchain>{
		new VulkanSwapchain{shared_from_this(), swapchainHandle, extent, format}};
}

void VulkanDevice::destroySwapchain(VkSwapchainKHR swapchainHandle) const
{
	vkDestroySwapchainKHR(device, swapchainHandle, nullptr);
}

std::vector<std::shared_ptr<VulkanImage>> VulkanDevice::getSwapchainImages(
	VkSwapchainKHR swapchain) const
{
	uint32_t imageCount;
	CHECK_VK_ERROR(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));
	std::vector<std::shared_ptr<VulkanImage>> ret{imageCount};
	std::vector<VkImage> swapchainImageHandles{imageCount};
	CHECK_VK_ERROR(
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImageHandles.data()));
	std::ranges::transform(swapchainImageHandles, ret.begin(), [&](VkImage image) {
		return std::shared_ptr<VulkanImage>{new VulkanImage{shared_from_this(), image, true}};
	});
	return ret;
}

void VulkanDevice::destroyImage(VkImage image) const
{
	vkDestroyImage(device, image, nullptr);
}

std::shared_ptr<VulkanImageView> VulkanDevice::createImageView(
	const VkImageViewCreateInfo& ci) const
{
	VkImageView viewHandle;
	CHECK_VK_ERROR(vkCreateImageView(device, &ci, nullptr, &viewHandle));
	return std::shared_ptr<VulkanImageView>{new VulkanImageView{shared_from_this(), viewHandle}};
}

void VulkanDevice::destroyImageView(VkImageView imageView) const
{
	vkDestroyImageView(device, imageView, nullptr);
}
