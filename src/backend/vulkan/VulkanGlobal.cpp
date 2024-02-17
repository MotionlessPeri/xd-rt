//
// Created by Frank on 2024/1/9.
//

#include "VulkanGlobal.h"
#include "MaterialFactoryVk.h"
#include "ModelFactoryVk.h"
#include "SamplerCache.h"
#include "TextureFactoryVk.h"
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
						VkPhysicalDeviceFeatures enabledFeatures,
						std::vector<const char*> deviceExtensions)
{
	std::call_once(initFlag, [&]() {
		instance = std::make_shared<VulkanInstance>(std::move(instanceEnabledExtensions),
													std::move(instanceEnabledLayers));
		physicalDevice =
			instance->pickPhysicalDevice([&](std::shared_ptr<VulkanPhysicalDevice> physicalDevice) {
				const VkPhysicalDeviceProperties2 deviceProperties =
					physicalDevice->getPhysicalDeviceProperties2();
				const VkBaseOutStructure* ptr =
					static_cast<VkBaseOutStructure*>(deviceProperties.pNext);
				while (ptr != nullptr &&
					   ptr->sType !=
						   VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEPTH_STENCIL_RESOLVE_PROPERTIES) {
					ptr = ptr->pNext;
				}
				if (ptr == nullptr)
					return false;
				const VkPhysicalDeviceDepthStencilResolveProperties& depthResolveProps =
					reinterpret_cast<const VkPhysicalDeviceDepthStencilResolveProperties&>(*ptr);
				const VkPhysicalDeviceFeatures deviceFeatures =
					physicalDevice->getPhysicalDeviceFeatures();
				const VkBool32* compareSupported =
					reinterpret_cast<const VkBool32*>(&deviceFeatures);
				const VkBool32* compareWanted = reinterpret_cast<const VkBool32*>(&enabledFeatures);
				for (const auto i :
					 std::views::iota(0ull, sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32))) {
					if (compareSupported[i] == 0 && compareWanted[i] == 1) {
						assert(false);
						return false;
					}
				}
				return deviceProperties.properties.deviceType ==
						   VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
					   ((depthResolveProps.supportedDepthResolveModes &
						 VK_RESOLVE_MODE_SAMPLE_ZERO_BIT) != 0);
			});

		VkSurfaceKHR surfaceHandle;
		SurfaceCIType surfaceCi;
		if (needPresent) {
			surfaceCi.pNext = nullptr;
			surfaceCi.flags = 0;
			surfaceCi.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surfaceCi.hwnd = handle;
			surfaceCi.hinstance = GetModuleHandle(nullptr);
			surfaceHandle = instance->createRawSurface(surfaceCi);
		}
		const auto graphicFamilyIndex = physicalDevice->getSuitableQueueFamilyIndex(
			[](VkPhysicalDevice device, const VkQueueFamilyProperties& properties,
			   int index) -> bool { return properties.queueFlags & (VK_QUEUE_GRAPHICS_BIT); });

		std::unordered_set<uint32_t> queueFamilyIndexesDup;

		queueFamilyIndexesDup.insert(graphicFamilyIndex);

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
			queueFamilyIndexesDup.insert(presentFamilyIndex);
		}
		const auto transferQueueIndex = physicalDevice->getSuitableQueueFamilyIndex(
			[](VkPhysicalDevice device, const VkQueueFamilyProperties& properties,
			   int index) -> bool { return properties.queueFlags & (VK_QUEUE_TRANSFER_BIT); });
		queueFamilyIndexesDup.insert(transferQueueIndex);

		const auto computeQueueIndex = physicalDevice->getSuitableQueueFamilyIndex(
			[](VkPhysicalDevice device, const VkQueueFamilyProperties& properties,
			   int index) -> bool { return properties.queueFlags & (VK_QUEUE_COMPUTE_BIT); });
		queueFamilyIndexesDup.insert(computeQueueIndex);

		DeviceDesc deviceDesc;
		deviceDesc.enabledExtensionNames = std::move(deviceExtensions);
		std::vector<VkDeviceQueueCreateInfo> queueCis;
		for (const auto queueFamilyIndex : queueFamilyIndexesDup) {
			deviceDesc.deviceQueueDescs.emplace_back();
			DeviceQueueDesc& desc = deviceDesc.deviceQueueDescs.back();
			desc.priorities = std::vector<float>(1u, 1.f);
			desc.ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			desc.ci.pNext = nullptr;
			desc.ci.flags = 0;
			desc.ci.queueFamilyIndex = queueFamilyIndex;
			desc.ci.queueCount = desc.priorities.size();
			desc.ci.pQueuePriorities = desc.priorities.data();
			queueCis.emplace_back(desc.ci);
		}
		deviceDesc.ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceDesc.ci.pNext = nullptr;
		deviceDesc.ci.flags = 0;
		deviceDesc.ci.queueCreateInfoCount = queueCis.size();
		deviceDesc.ci.pQueueCreateInfos = queueCis.data();
		deviceDesc.ci.enabledLayerCount = 0;
		deviceDesc.ci.ppEnabledLayerNames = nullptr;
		deviceDesc.ci.enabledExtensionCount = deviceDesc.enabledExtensionNames.size();
		deviceDesc.ci.ppEnabledExtensionNames = deviceDesc.enabledExtensionNames.data();
		deviceDesc.ci.pEnabledFeatures = nullptr;
		device = physicalDevice->createLogicalDevice(deviceDesc);

		graphicQueue = device->getQueue(graphicFamilyIndex, 0);
		computeQueue = device->getQueue(computeQueueIndex, 0);
		transferQueue = device->getQueue(transferQueueIndex, 0);
		if (needPresent) {
			presentQueue = device->getQueue(presentFamilyIndex, 0);
			surface = device->createSurface(surfaceCi, surfaceHandle);
			swapchain = surface->createSwapchain(width, height);
		}

		ModelFactoryVk::init(device);
		TextureFactoryVk::init(device);
		SamplerCache::init(device);
		MaterialFactoryVk::init(device);
	});
}

void VulkanGlobal::terminate()
{
	ModelFactoryVk::terminate();
	TextureFactoryVk::terminate();
	SamplerCache::terminate();
	MaterialFactoryVk::terminate();
}
