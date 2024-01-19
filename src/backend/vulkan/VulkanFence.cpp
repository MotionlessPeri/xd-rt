//
// Created by Frank on 2024/1/18.
//

#include "VulkanFence.h"

#include "VulkanDevice.h"
using namespace xd;
VulkanFence::~VulkanFence()
{
	device->destroyFence(fence);
}

void VulkanFence::wait() const
{
	device->waitForFences({fence});
}
void VulkanFence::reset() const
{
	device->resetFences({fence});
}

VulkanFence::VulkanFence(std::shared_ptr<const VulkanDevice> device,
						 const VkFenceCreateInfo& desc,
						 VkFence fence)
	: VulkanDeviceObject(std::move(device), desc), fence(fence)
{
}
