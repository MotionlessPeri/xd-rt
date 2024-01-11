//
// Created by Frank on 2024/1/9.
//

#ifndef XD_RT_VULKANGLOBAL_H
#define XD_RT_VULKANGLOBAL_H
#include <memory>
#include <mutex>
#include <vector>
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {
static std::once_flag initFlag;
class VulkanGlobal {
public:
	static void init(std::vector<const char*> instanceEnabledExtensions,
					 std::vector<const char*> instanceEnabledLayers,
					 bool needPresent,
					 NativeWindowType handle,
					 int width,
					 int height,
					 VkPhysicalDeviceFeatures features,
					 std::vector<const char*> deviceExtensions);

	inline static std::shared_ptr<VulkanInstance> instance = nullptr;
	inline static std::shared_ptr<VulkanPhysicalDevice> physicalDevice = nullptr;
	inline static std::shared_ptr<VulkanDevice> device = nullptr;
	inline static std::shared_ptr<VulkanSurface> surface = nullptr;
	inline static std::shared_ptr<VulkanSwapchain> swapchain = nullptr;
	inline static std::shared_ptr<VulkanQueue> graphicQueue = nullptr;
	inline static std::shared_ptr<VulkanQueue> presentQueue = nullptr;
};

}  // namespace xd

#endif	// XD_RT_VULKANGLOBAL_H
