//
// Created by Frank on 2024/1/14.
//

#ifndef XD_RT_VULKANCOMMANDBUFFER_H
#define XD_RT_VULKANCOMMANDBUFFER_H
#include <vector>

#include "VulkanDescs.h"
#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
namespace xd {

class VulkanCommandBuffer : public VulkanDeviceObject<VkCommandBufferAllocateInfo> {
public:
	friend class VulkanDevice;
	friend class VulkanCommandPool;
	VulkanCommandBuffer() = delete;
	VulkanCommandBuffer(const VulkanCommandBuffer& other) = delete;
	VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept = delete;
	VulkanCommandBuffer& operator=(const VulkanCommandBuffer& other) = delete;
	VulkanCommandBuffer& operator=(VulkanCommandBuffer&& other) noexcept = delete;
	~VulkanCommandBuffer();
	void beginCommandBuffer(const VkCommandBufferBeginInfo& info) const;
	void endCommandBuffer() const;

	void copyBuffers(VkBuffer srcBuffer,
					 VkBuffer dstBuffer,
					 uint32_t regionCount,
					 VkBufferCopy&& pRegions) const;

	void beginRenderPass(const VkRenderPassBeginInfo& info, VkSubpassContents subpass) const;
	void endRenderPass() const;

	void bindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) const;

	void setViewport(const VkViewport& viewport) const;
	void setScissor(const VkRect2D& scissor) const;
	void bindVertexBuffer(uint32_t bindingPoint, VkBuffer buffer) const
	{
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(cmdBuffer, bindingPoint, 1, &buffer, offsets);
	}
	void bindVertexBuffers(uint32_t firstBinding,
						   const std::vector<VkBuffer>& buffers,
						   const std::vector<VkDeviceSize>& offsets) const
	{
		vkCmdBindVertexBuffers(cmdBuffer, firstBinding, buffers.size(), buffers.data(),
							   offsets.data());
	}
	void bindIndexBuffer(VkBuffer buffer, VkIndexType indexType) const
	{
		vkCmdBindIndexBuffer(cmdBuffer, buffer, 0, indexType);
	}
	void draw(uint32_t vertexCount,
			  uint32_t instanceCount,
			  uint32_t firstVertex,
			  uint32_t firstInstance) const;

	void drawIndexed(uint32_t indexCount,
					 uint32_t instanceCount,
					 uint32_t firstIndex,
					 int32_t vertexOffset,
					 uint32_t firstInstance) const;

	void submitAndWait() const;
	void submit(const SubmitInfoContainer& container) const;
	void reset() const;

private:
	VulkanCommandBuffer(std::shared_ptr<const VulkanDevice> device,
						const VkCommandBufferAllocateInfo& desc,
						std::shared_ptr<const VulkanCommandPool> pool,
						VkCommandBuffer cmd_buffer);
	std::shared_ptr<const VulkanCommandPool> pool = nullptr;
	VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
};

}  // namespace xd

#endif	// XD_RT_VULKANCOMMANDBUFFER_H
