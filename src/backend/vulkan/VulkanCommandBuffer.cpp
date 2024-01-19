//
// Created by Frank on 2024/1/14.
//

#include "VulkanCommandBuffer.h"
#include <ranges>
#include "VulkanCommandPool.h"
#include "VulkanDevice.h"
#include "VulkanMacros.h"
#include "VulkanQueue.h"
#include "VulkanSemaphore.h"
using namespace xd;
VulkanCommandBuffer::VulkanCommandBuffer(std::shared_ptr<const VulkanDevice> device,
										 const VkCommandBufferAllocateInfo& desc,
										 std::shared_ptr<const VulkanCommandPool> pool,
										 VkCommandBuffer cmd_buffer)
	: VulkanDeviceObject(std::move(device), desc), pool(std::move(pool)), cmdBuffer(cmd_buffer)
{
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
	device->freeCommandBuffer(pool->pool, cmdBuffer);
}

void VulkanCommandBuffer::beginCommandBuffer(const VkCommandBufferBeginInfo& info) const
{
	CHECK_VK_ERROR(vkBeginCommandBuffer(cmdBuffer, &info));
}

void VulkanCommandBuffer::endCommandBuffer() const
{
	CHECK_VK_ERROR(vkEndCommandBuffer(cmdBuffer));
}

void VulkanCommandBuffer::copyBuffers(VkBuffer srcBuffer,
									  VkBuffer dstBuffer,
									  uint32_t regionCount,
									  VkBufferCopy&& pRegions) const
{
	vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, regionCount, &pRegions);
}

void VulkanCommandBuffer::beginRenderPass(const VkRenderPassBeginInfo& info,
										  VkSubpassContents subpass) const
{
	vkCmdBeginRenderPass(cmdBuffer, &info, subpass);
}

void VulkanCommandBuffer::endRenderPass() const
{
	vkCmdEndRenderPass(cmdBuffer);
}

void VulkanCommandBuffer::bindPipeline(VkPipelineBindPoint pipelineBindPoint,
									   VkPipeline pipeline) const
{
	vkCmdBindPipeline(cmdBuffer, pipelineBindPoint, pipeline);
}

void VulkanCommandBuffer::setViewport(const VkViewport& viewport) const
{
	vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
}

void VulkanCommandBuffer::setScissor(const VkRect2D& scissor) const
{
	vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
}

void VulkanCommandBuffer::draw(uint32_t vertexCount,
							   uint32_t instanceCount,
							   uint32_t firstVertex,
							   uint32_t firstInstance) const
{
	vkCmdDraw(cmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandBuffer::drawIndexed(uint32_t indexCount,
									  uint32_t instanceCount,
									  uint32_t firstIndex,
									  int32_t vertexOffset,
									  uint32_t firstInstance) const
{
	vkCmdDrawIndexed(cmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandBuffer::submitAndWait() const
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;
	pool->getQueue()->submitAndWait({submitInfo}, nullptr);
}

void VulkanCommandBuffer::submit(const SubmitInfoContainer& container) const
{
	const auto semaphoreToHandle =
		[](const std::vector<std::shared_ptr<VulkanSemaphore>>& vec) -> std::vector<VkSemaphore> {
		const auto handleView = vec | std::views::transform([&](const auto& semaphore) {
									return semaphore->semaphore;
								});
		return {handleView.begin(), handleView.end()};
	};
	const auto waitingHandles = semaphoreToHandle(container.waitingSemaphores);
	const auto signalingHandles = semaphoreToHandle(container.signalingSemaphores);
	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = waitingHandles.size();
	submitInfo.pWaitSemaphores = waitingHandles.data();
	submitInfo.pWaitDstStageMask = container.waitingStages.data();
	submitInfo.commandBufferCount = 1;
	submitInfo.signalSemaphoreCount = signalingHandles.size();
	submitInfo.pSignalSemaphores = signalingHandles.data();
	submitInfo.pCommandBuffers = &cmdBuffer;
	pool->getQueue()->submit({submitInfo}, container.signalingFence);
}

void VulkanCommandBuffer::reset() const
{
	vkResetCommandBuffer(cmdBuffer, 0);
}
