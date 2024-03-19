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
	friend class ImguiAppBase;	// TODO: find a better way to encapsulate imgui, maybe in the lib?
	friend class BasicFramegraphApp;
	VulkanCommandBuffer() = delete;
	VulkanCommandBuffer(const VulkanCommandBuffer& other) = delete;
	VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept = delete;
	VulkanCommandBuffer& operator=(const VulkanCommandBuffer& other) = delete;
	VulkanCommandBuffer& operator=(VulkanCommandBuffer&& other) noexcept = delete;
	~VulkanCommandBuffer();
	void beginCommandBuffer(const VkCommandBufferBeginInfo& info) const;
	void endCommandBuffer() const;
	void pipelineBarrier(VkPipelineStageFlags srcStageMask,
						 VkPipelineStageFlags dstStageMask,
						 VkDependencyFlags dependencyFlags,
						 const std::vector<VkMemoryBarrier>& memoryBarriers,
						 const std::vector<VkBufferMemoryBarrier>& bufferMemoryBarriers,
						 const std::vector<VkImageMemoryBarrier>& imageMemoryBarriers) const;
	void pipelineBarrier2(const VkDependencyInfo& info) const;
	void copyBuffers(VkBuffer srcBuffer,
					 VkBuffer dstBuffer,
					 uint32_t regionCount,
					 VkBufferCopy&& pRegions) const;
	void copyBufferToImage(VkBuffer srcBuffer,
						   VkImage dstImage,
						   VkImageLayout dstImageLayout,
						   const VkBufferImageCopy& region) const;
	void beginRenderPass(const VkRenderPassBeginInfo& info, VkSubpassContents subpass) const;
	void nextSubpass(VkSubpassContents subpass) const;
	void endRenderPass() const;

	void bindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline) const;
	void setViewport(const VkViewport& viewport) const;
	void setScissor(const VkRect2D& scissor) const;
	void bindDescriptorSets(VkPipelineBindPoint bindPoint,
							VkPipelineLayout layout,
							uint32_t firstSet,
							const std::vector<VkDescriptorSet>& descriptorSets) const;

	void bindVertexBuffer(uint32_t bindingPoint, VkBuffer buffer) const;

	void bindVertexBuffers(uint32_t firstBinding,
						   const std::vector<VkBuffer>& buffers,
						   const std::vector<VkDeviceSize>& offsets) const;

	void bindIndexBuffer(VkBuffer buffer, VkIndexType indexType) const;
	void draw(uint32_t vertexCount,
			  uint32_t instanceCount,
			  uint32_t firstVertex,
			  uint32_t firstInstance) const;

	void drawIndexed(uint32_t indexCount,
					 uint32_t instanceCount,
					 uint32_t firstIndex,
					 int32_t vertexOffset,
					 uint32_t firstInstance) const;
	void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const;
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
