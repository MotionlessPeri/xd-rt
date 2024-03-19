//
// Created by Frank on 2024/1/24.
//

#ifndef XD_RT_TEXTUREVK_H
#define XD_RT_TEXTUREVK_H
#include <memory>

#include "VulkanImageView.h"
#include "VulkanPlatformSpecific.h"
#include "VulkanSampler.h"
#include "VulkanTypes.h"
namespace xd {

class TextureVk {
public:
	VkDescriptorImageInfo getBindingInfo(VkImageLayout layout) const
	{
		VkDescriptorImageInfo ret;
		ret.imageView = imageView->imageView;
		ret.imageLayout = layout;
		ret.sampler = sampler ? sampler->sampler : VK_NULL_HANDLE;
		return ret;
	}
	std::shared_ptr<VulkanImage> image;
	std::shared_ptr<VulkanImageView> imageView;
	std::shared_ptr<VulkanSampler> sampler;
};

}  // namespace xd

#endif	// XD_RT_TEXTUREVK_H
