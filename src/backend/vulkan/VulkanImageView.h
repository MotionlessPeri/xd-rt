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
	friend class TextureVk;
	VulkanImageView() = delete;
	VulkanImageView(const VulkanImageView& other) = delete;
	VulkanImageView(VulkanImageView&& other) noexcept = delete;
	VulkanImageView& operator=(const VulkanImageView& other) = delete;
	VulkanImageView& operator=(VulkanImageView&& other) noexcept = delete;
	~VulkanImageView();

private:
	VulkanImageView(std::shared_ptr<const VulkanDevice> device,
					const VkImageViewCreateInfo& desc,
					std::shared_ptr<const VulkanImage> image_ref,
					VkImageView image_view);

	std::shared_ptr<const VulkanImage> imageRef = nullptr;
	VkImageView imageView;
};

}  // namespace xd

#endif	// XD_RT_VULKANIMAGEVIEW_H
