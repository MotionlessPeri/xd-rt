//
// Created by Frank on 2024/1/13.
//

#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanGlobal.h"
#include "VulkanQueue.h"
using namespace xd;

void VulkanBuffer::copyToImage(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
							   VkImage dstImage,
							   VkImageLayout dstImageLayout,
							   const VkBufferImageCopy& region) const
{
	cmdBuffer->copyBufferToImage(buffer, dstImage, dstImageLayout, region);
}

void VulkanBuffer::transitState(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
								VkPipelineStageFlags srcStageMask,
								VkPipelineStageFlags dstStageMask,
								VkBufferMemoryBarrier&& bufferBarrier) const
{
	bufferBarrier.buffer = buffer;
	cmdBuffer->pipelineBarrier(srcStageMask, dstStageMask, 0, {}, {bufferBarrier}, {});
}

VulkanBuffer::VulkanBuffer(std::shared_ptr<const VulkanDevice> _device,
						   VkBufferCreateInfo _desc,
						   VkBuffer _buffer,
						   VkMemoryPropertyFlags properties)
	: VulkanDeviceObject(std::move(_device), _desc), buffer(_buffer)
{
	const VkMemoryRequirements memRequirements = device->getBufferMemoryRequirements(buffer);
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	memory = device->allocateMemory(std::move(allocInfo), memRequirements, properties);
	device->bindBufferMemory(buffer, memory->memory, 0u);
}
VulkanBuffer::~VulkanBuffer()
{
	device->destroyBuffer(buffer);
}

void VulkanBuffer::setData(uint32_t offset, const void* ptr, uint32_t size) const
{
	const auto memoryType = device->getMemoryType(memory->desc.memoryTypeIndex);
	if ((memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {
		// memory can be accessed by host(CPU)
		memory->map(offset, ptr, size);
	}
	else {
		// memory can not be directly accessed by host(CPU), a staging buffer is required
		VkBufferCreateInfo stagingBufferCi = desc;
		stagingBufferCi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		const auto stagingBuffer =
			device->createBuffer(std::move(stagingBufferCi), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		stagingBuffer->setData(offset, ptr, size);
		// Copy staging buffer contents to origin buffer
		const auto transferQueue = VulkanGlobal::transferQueue;
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = transferQueue->getQueueFamilyIndex();
		const auto cmdPool = device->createCommandPool(std::move(poolInfo));

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		const auto cmdBuffer = cmdPool->allocateCommandBuffers(std::move(allocInfo))[0];

		cmdBuffer->beginCommandBuffer({VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
									   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr});
		cmdBuffer->copyBuffers(stagingBuffer->buffer, buffer, 1, {0, 0, size});
		cmdBuffer->endCommandBuffer();
		cmdBuffer->submitAndWait();
	}
}
