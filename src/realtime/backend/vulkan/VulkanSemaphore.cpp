//
// Created by Frank on 2024/1/18.
//

#include "VulkanSemaphore.h"

#include "VulkanDevice.h"
using namespace xd;
VulkanSemaphore::~VulkanSemaphore()
{
	device->destroySemaphore(semaphore);
}

VulkanSemaphore::VulkanSemaphore(std::shared_ptr<const VulkanDevice> device,
								 const VkSemaphoreCreateInfo& desc,
								 VkSemaphore semaphore)
	: VulkanDeviceObject(std::move(device), desc), semaphore(semaphore)
{
}
