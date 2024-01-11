//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_VULKANIMAGEVIEW_H
#define XD_RT_VULKANIMAGEVIEW_H

#include <memory>
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"

namespace xd {

class VulkanImageView {
public:
	friend class VulkanDevice;
	VulkanImageView() = delete;
	VulkanImageView(const VulkanImageView& other) = delete;
	VulkanImageView(VulkanImageView&& other) noexcept = delete;
	VulkanImageView& operator=(const VulkanImageView& other) = delete;
	VulkanImageView& operator=(VulkanImageView&& other) noexcept = delete;
	~VulkanImageView();

private:
	explicit VulkanImageView(std::weak_ptr<const VulkanDevice> device, VkImageView image_view);
	std::weak_ptr<const VulkanDevice> deviceWeakRef;
	VkImageView imageView;
};

}  // namespace xd

#endif	// XD_RT_VULKANIMAGEVIEW_H
