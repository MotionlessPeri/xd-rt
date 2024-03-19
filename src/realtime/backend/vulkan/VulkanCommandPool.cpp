//
// Created by Frank on 2024/1/14.
//

#include "VulkanCommandPool.h"

#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
using namespace xd;
VulkanCommandPool::~VulkanCommandPool()
{
	device->destroyCommandPool(pool);
}

std::vector<std::shared_ptr<VulkanCommandBuffer>> VulkanCommandPool::allocateCommandBuffers(
	VkCommandBufferAllocateInfo&& ai) const
{
	ai.commandPool = pool;
	return device->createCommandBuffers(std::move(ai), shared_from_this());
}

void VulkanCommandPool::freeCommandBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const
{
	device->freeCommandBuffer(pool, cmdBuffer->cmdBuffer);
}

void VulkanCommandPool::freeCommandBuffers(
	const std::vector<std::shared_ptr<VulkanCommandBuffer>>& cmdBuffers) const
{
	std::vector<VkCommandBuffer> handles(cmdBuffers.size());
	std::ranges::transform(
		cmdBuffers, handles.begin(),
		[](const std::shared_ptr<VulkanCommandBuffer>& ptr) { return ptr->cmdBuffer; });
	device->freeCommandBuffers(pool, handles);
}

VulkanCommandPool::VulkanCommandPool(std::shared_ptr<const VulkanDevice> device,
									 const VkCommandPoolCreateInfo& desc,
									 std::shared_ptr<VulkanQueue> queue,
									 VkCommandPool pool)
	: VulkanDeviceObject(std::move(device), desc), queue(std::move(queue)), pool(pool)
{
}
