//
// Created by Frank on 2024/1/11.
//

#ifndef XD_RT_VULKANDEVICEOBJECT_H
#define XD_RT_VULKANDEVICEOBJECT_H

#include <memory>
#include "VulkanTypes.h"
namespace xd {
template <typename ObjectDescType>
class VulkanDeviceObject {
public:
	explicit VulkanDeviceObject(std::shared_ptr<const VulkanDevice> device)
		: device(std::move(device))
	{
	}

	VulkanDeviceObject(std::shared_ptr<const VulkanDevice> device, const ObjectDescType& desc)
		: device(std::move(device)), desc(std::move(desc))
	{
	}

	VulkanDeviceObject() = delete;
	VulkanDeviceObject(const VulkanDeviceObject& other) = delete;
	VulkanDeviceObject(VulkanDeviceObject&& other) noexcept = delete;
	VulkanDeviceObject& operator=(const VulkanDeviceObject& other) = delete;
	VulkanDeviceObject& operator=(VulkanDeviceObject&& other) noexcept = delete;

	const auto& getDesc() const { return desc; }  // Note: for test purpose only

protected:
	std::shared_ptr<const VulkanDevice> device;
	ObjectDescType desc{};
};

}  // namespace xd

#endif	// XD_RT_VULKANDEVICEOBJECT_H
