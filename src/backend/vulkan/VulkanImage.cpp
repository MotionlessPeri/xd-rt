//
// Created by Frank on 2024/1/10.
//

#include "VulkanImage.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanGlobal.h"
#include "VulkanQueue.h"
using namespace xd;
VulkanImage::VulkanImage(std::shared_ptr<const VulkanDevice> _device, VkImage image)
	: VulkanDeviceObject(std::move(_device)), image(image), isSwapchainImage(true)
{
}

VulkanImage::VulkanImage(std::shared_ptr<const VulkanDevice> _device,
						 VkImageCreateInfo _desc,
						 VkImage image,
						 VkMemoryPropertyFlags properties)
	: VulkanDeviceObject(std::move(_device), std::move(_desc)),
	  image(image),
	  isSwapchainImage(false)
{
	const auto memReq = device->getImageMemoryRequirements(image);
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	memory = device->allocateMemory(std::move(allocInfo), memReq, properties);
	device->bindImageMemory(image, memory->memory, 0u);
}

VulkanImage::~VulkanImage()
{
	if (!isSwapchainImage)
		device->destroyImage(image);
}

std::shared_ptr<VulkanImageView> VulkanImage::createImageView(VkImageViewCreateInfo&& ci) const
{
	ci.image = image;
	return device->createImageView(shared_from_this(), ci);
}

void VulkanImage::setData(uint32_t offset, void* ptr, uint32_t size) const
{
	const auto memoryType = device->getMemoryType(memory->desc.memoryTypeIndex);
	if ((memoryType.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) {
		// memory can be accessed by host(CPU)
		memory->map(offset, ptr, size);
	}
	else {
		// memory can not be directly accessed by host(CPU), a staging buffer is required
		VkBufferCreateInfo stagingBufferCi;
		stagingBufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferCi.pNext = nullptr;
		stagingBufferCi.flags = 0;
		stagingBufferCi.size = memory->desc.allocationSize;
		stagingBufferCi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingBufferCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		stagingBufferCi.queueFamilyIndexCount = 0;
		stagingBufferCi.pQueueFamilyIndices = nullptr;
		const auto stagingBuffer =
			device->createBuffer(std::move(stagingBufferCi), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		stagingBuffer->setData(offset, ptr, size);
		// Copy staging buffer contents to origin image
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
		const auto cmdBuffer = cmdPool->allocateCommandBuffers(std::move(allocInfo)).front();
		cmdBuffer->beginCommandBuffer({VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
									   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr});

		const auto imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;	 // TODO: determine this by other infos
		VkBufferImageCopy copyDesc;
		copyDesc.bufferOffset = 0;
		copyDesc.bufferRowLength = desc.extent.width;
		copyDesc.bufferImageHeight = desc.extent.height;
		copyDesc.imageSubresource.aspectMask = imageAspect;
		copyDesc.imageSubresource.mipLevel = 0;
		copyDesc.imageSubresource.baseArrayLayer = 0;
		copyDesc.imageSubresource.layerCount = 1;
		copyDesc.imageOffset = {0, 0, 0};
		copyDesc.imageExtent = desc.extent;
		stagingBuffer->copyToImage(cmdBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
								   copyDesc);
		cmdBuffer->endCommandBuffer();
		cmdBuffer->submitAndWait();
	}
}

void VulkanImage::transitState(VkPipelineStageFlags srcStageMask,
							   VkPipelineStageFlags dstStageMask,
							   VkImageMemoryBarrier&& imageBarrier) const
{
	imageBarrier.image = image;
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
	const auto cmdBuffer = cmdPool->allocateCommandBuffers(std::move(allocInfo)).front();
	cmdBuffer->beginCommandBuffer({VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
								   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr});
	cmdBuffer->pipelineBarrier(srcStageMask, dstStageMask, 0, {}, {}, {imageBarrier});
	cmdBuffer->endCommandBuffer();
	cmdBuffer->submitAndWait();
}
