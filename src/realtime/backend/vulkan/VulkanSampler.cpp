//
// Created by Frank on 2024/1/24.
//

#include "VulkanSampler.h"
#include "VulkanDevice.h"
using namespace xd;

VulkanSampler::~VulkanSampler()
{
	device->destroySampler(sampler);
}

VulkanSampler::VulkanSampler(std::shared_ptr<const VulkanDevice> device,
							 const SamplerDesc& desc,
							 VkSampler sampler)
	: VulkanDeviceObject(std::move(device), desc), sampler(sampler)
{
}
