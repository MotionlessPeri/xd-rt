//
// Created by Frank on 2024/1/9.
//

#ifndef XD_RT_VULKANINSTANCE_H
#define XD_RT_VULKANINSTANCE_H
#include <algorithm>
#include <memory>
#include <mutex>
#include <vector>
#include "VulkanMacros.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {
static std::once_flag initPhysicalDeviceFlag;
class VulkanInstance : public std::enable_shared_from_this<VulkanInstance> {
public:
	friend class VulkanPhysicalDevice;
	friend class VulkanDevice;
	friend class VulkanGlobal;
	friend class ImguiAppBase;	// TODO: find a better way to encapsulate imgui, maybe in the lib?
	VulkanInstance() = delete;
	VulkanInstance(const VulkanInstance& other) = delete;
	VulkanInstance(VulkanInstance&& other) noexcept = delete;
	VulkanInstance& operator=(const VulkanInstance& other) = delete;
	VulkanInstance& operator=(VulkanInstance&& other) noexcept = delete;
	VulkanInstance(std::vector<const char*>&& enabledExtensions,
				   std::vector<const char*>&& enabledLayers);
	~VulkanInstance();
	template <typename Func>
	std::shared_ptr<VulkanPhysicalDevice> pickPhysicalDevice(Func func)
	{
		std::call_once(initPhysicalDeviceFlag, [&]() {
			uint32_t deviceCount = 0;
			CHECK_VK_ERROR(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr));
			if (deviceCount == 0) {
				throw std::runtime_error{""};
			}

			std::vector<VkPhysicalDevice> container{deviceCount};
			CHECK_VK_ERROR(vkEnumeratePhysicalDevices(instance, &deviceCount, container.data()));

			physicalDevices.resize(deviceCount);
			std::ranges::transform(
				container, physicalDevices.begin(),
				[&](VkPhysicalDevice device) -> std::shared_ptr<VulkanPhysicalDevice> {
					return std::shared_ptr<VulkanPhysicalDevice>{
						new VulkanPhysicalDevice{device, shared_from_this()}};
				});
		});
		for (const auto& pDevice : physicalDevices) {
			if (func(pDevice))
				return pDevice;
		}
		return nullptr;
	}

private:
	VkSurfaceKHR createRawSurface(const SurfaceCIType& ci) const;
	VkInstance instance = VK_NULL_HANDLE;
	VkApplicationInfo appInfo{};
	VkInstanceCreateInfo createInfo{};
	VkDebugUtilsMessengerEXT debugMessenger;
	std::vector<std::shared_ptr<VulkanPhysicalDevice>> physicalDevices{};
};

}  // namespace xd

#endif	// XD_RT_VULKANINSTANCE_H
