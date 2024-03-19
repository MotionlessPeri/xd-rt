//
// Created by Frank on 2024/1/24.
//

#ifndef XD_RT_VULKANSAMPLER_H
#define XD_RT_VULKANSAMPLER_H
#include "VulkanDescs.h"
#include "VulkanDeviceObject.h"
#include "VulkanTypes.h"
namespace xd {

class VulkanSampler : public VulkanDeviceObject<SamplerDesc> {
public:
	friend class VulkanDevice;
	friend class SamplerCache;
	friend class TextureVk;
	VulkanSampler() = delete;
	VulkanSampler(const VulkanSampler& other) = delete;
	VulkanSampler(VulkanSampler&& other) noexcept = delete;
	VulkanSampler& operator=(const VulkanSampler& other) = delete;
	VulkanSampler& operator=(VulkanSampler&& other) noexcept = delete;
	~VulkanSampler();

private:
	VulkanSampler(std::shared_ptr<const VulkanDevice> device,
				  const SamplerDesc& desc,
				  VkSampler sampler);

	VkSampler sampler = VK_NULL_HANDLE;
};

}  // namespace xd

#endif	// XD_RT_VULKANSAMPLER_H
