//
// Created by Frank on 2024/1/9.
//

#include "VulkanGlobal.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanSurface.h"
using namespace xd;

void VulkanGlobal::init(std::vector<const char*> instanceEnabledExtensions,
						std::vector<const char*> instanceEnabledLayers,
						bool needPresent,
						NativeWindowType handle,
						int width,
						int height,
						VkPhysicalDeviceFeatures features,
						std::vector<const char*> deviceExtensions)
{
	std::call_once(initFlag, [&]() {
		instance = std::make_shared<VulkanInstance>(std::move(instanceEnabledExtensions),
													std::move(instanceEnabledLayers));
		physicalDevice =
			instance->pickPhysicalDevice([](std::shared_ptr<VulkanPhysicalDevice> physicalDevice) {
				const VkPhysicalDeviceProperties deviceProperties =
					physicalDevice->getPhysicalDeviceProperties();
				const VkPhysicalDeviceFeatures deviceFeatures =
					physicalDevice->getPhysicalDeviceFeatures();
				return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
					   deviceFeatures.geometryShader;
			});

		VkSurfaceKHR surfaceHandle;
		if (needPresent) {
			SurfaceCIType ci;
			ci.pNext = nullptr;
			ci.flags = 0;
			ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			ci.hwnd = handle;
			ci.hinstance = GetModuleHandle(nullptr);
			surfaceHandle = instance->createRawSurface(ci);
		}
		const auto graphicFamilyIndex = physicalDevice->getSuitableQueueFamilyIndex(
			[](VkPhysicalDevice device, const VkQueueFamilyProperties& properties,
			   int index) -> bool { return properties.queueFlags & (VK_QUEUE_GRAPHICS_BIT); });
		std::vector<uint32_t> queueFamilyIndexes;
		constexpr uint32_t queueCnt = 1u;
		std::vector<std::vector<float>> queuePriorities;
		const auto addQueueFamily = [&](int familyIndex) -> void {
			queueFamilyIndexes.emplace_back(familyIndex);
			queuePriorities.emplace_back(std::vector<float>{queueCnt, 1.f});
		};
		addQueueFamily(graphicFamilyIndex);

		int presentFamilyIndex;
		if (needPresent) {
			presentFamilyIndex = physicalDevice->getSuitableQueueFamilyIndex(
				[&](VkPhysicalDevice device, const VkQueueFamilyProperties& properties,
					int index) -> bool {
					VkBool32 presentSupport = false;
					vkGetPhysicalDeviceSurfaceSupportKHR(device, index, surfaceHandle,
														 &presentSupport);
					return presentSupport;
				});
			if (presentFamilyIndex != graphicFamilyIndex)
				addQueueFamily(presentFamilyIndex);
		}
		device = physicalDevice->createLogicalDevice(std::move(queueFamilyIndexes),
													 std::move(queuePriorities), features,
													 std::move(deviceExtensions));
		graphicQueue = device->getQueue(graphicFamilyIndex, 0);
		if (needPresent)
			presentQueue = device->getQueue(presentFamilyIndex, 0);
		if (needPresent) {
			surface = device->createSurface(surfaceHandle);
			swapchain = surface->createSwapchain(width, height);
		}
	});
}
