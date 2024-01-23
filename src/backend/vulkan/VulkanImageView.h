//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_VULKANIMAGEVIEW_H
#define XD_RT_VULKANIMAGEVIEW_H

#include <memory>

#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"

namespace xd {

class VulkanImageView : public VulkanDeviceObject<VkImageViewCreateInfo> {
public:
	friend class VulkanDevice;
	friend class VulkanGLFWApp;	 // TODO: I don't know if this works. Delete it ASAP
	friend class FrameGraph;
	VulkanImageView() = delete;
	VulkanImageView(const VulkanImageView& other) = delete;
	VulkanImageView(VulkanImageView&& other) noexcept = delete;
	VulkanImageView& operator=(const VulkanImageView& other) = delete;
	VulkanImageView& operator=(VulkanImageView&& other) noexcept = delete;
	~VulkanImageView();

private:
	VulkanImageView(std::shared_ptr<const VulkanDevice> device,
					VkImageViewCreateInfo desc,
					VkImageView image_view);
	VkImageView imageView;
};

}  // namespace xd

#endif	// XD_RT_VULKANIMAGEVIEW_H
