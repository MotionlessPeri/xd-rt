//
// Created by Frank on 2024/1/9.
//

#ifndef XD_RT_VULKANDEVICE_H
#define XD_RT_VULKANDEVICE_H
#include <unordered_map>
#include "MathUtil.h"
#include "VulkanDescs.h"
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {

class VulkanDevice : public std::enable_shared_from_this<VulkanDevice> {
public:
	friend class VulkanPhysicalDevice;
	VulkanDevice() = delete;
	VulkanDevice(const VulkanDevice& other) = delete;
	VulkanDevice(VulkanDevice&& other) noexcept = delete;
	VulkanDevice& operator=(const VulkanDevice& other) = delete;
	VulkanDevice& operator=(VulkanDevice&& other) noexcept = delete;
	~VulkanDevice();
	std::shared_ptr<VulkanQueue> getQueue(int queueFamilyIndex, int queueIndex);
	std::shared_ptr<VulkanQueue> getQueue(const std::pair<int, int>& key);
	// VkDevice getHandle() const { return device; }

	std::shared_ptr<VulkanSurface> createSurface(const SurfaceCIType& ci) const;
	std::shared_ptr<VulkanSurface> createSurface(const SurfaceCIType& ci,
												 VkSurfaceKHR surface) const;
	void destroySurface(VkSurfaceKHR surface) const;

	std::shared_ptr<VulkanSwapchain> createSwapchain(const VkSwapchainCreateInfoKHR& ci,
													 VkExtent2D extent,
													 VkSurfaceFormatKHR format) const;
	void destroySwapchain(VkSwapchainKHR swapchainHandle) const;

	std::vector<std::shared_ptr<VulkanImage>> getSwapchainImages(VkSwapchainKHR swapchain) const;
	void destroyImage(VkImage image) const;

	std::shared_ptr<VulkanImageView> createImageView(const VkImageViewCreateInfo& ci) const;
	void destroyImageView(VkImageView imageView) const;

	std::shared_ptr<VulkanShader> createShader(const VkShaderModuleCreateInfo& ci,
											   VkShaderStageFlagBits stage,
											   std::string entry_point_name = "main") const;
	void destroyShader(VkShaderModule shaderModuleHandle) const;

	std::shared_ptr<VulkanCommandPool> createCommandPool(VkCommandPoolCreateInfo&& ci) const;
	void destroyCommandPool(VkCommandPool pool) const;

	std::vector<std::shared_ptr<VulkanCommandBuffer>> createCommandBuffers(
		const VkCommandBufferAllocateInfo& ai,
		const std::shared_ptr<const VulkanCommandPool>& poolRef) const;
	void freeCommandBuffer(VkCommandPool pool, VkCommandBuffer cmdBuffer) const;
	void freeCommandBuffers(VkCommandPool pool,
							const std::vector<VkCommandBuffer>& cmdBuffers) const;

	std::shared_ptr<VulkanBuffer> createBuffer(const VkBufferCreateInfo& ci,
											   VkMemoryPropertyFlags properties) const;
	VkMemoryRequirements getBufferMemoryRequirements(VkBuffer buffer) const;
	void bindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) const;
	void destroyBuffer(VkBuffer bufferHandle) const;

	const VkMemoryType& getMemoryType(uint32_t index) const;

	std::unique_ptr<VulkanMemory> allocateMemory(VkMemoryAllocateInfo&& ai,
												 const VkMemoryRequirements& memRequirements,
												 VkMemoryPropertyFlags properties) const;
	void freeMemory(VkDeviceMemory memory) const;

	void* mapMemory(VkDeviceMemory memoryHandle, uint32_t offset, uint32_t size) const;
	void unmapMemory(VkDeviceMemory memoryHandle) const;

	void flushMemory(const std::vector<VkMappedMemoryRange>& ranges) const;

	std::shared_ptr<const VulkanPhysicalDevice> getPhysicalDevice() const { return physicalDevice; }

	std::shared_ptr<VulkanDescriptorSetLayout> createDescriptorSetLayout(
		const DescriptorSetLayoutDesc& desc) const;
	void destroyDescriptorSetLayout(VkDescriptorSetLayout layout) const;

	std::shared_ptr<VulkanDescriptorPool> createDescriptorPool(
		const DescriptorPoolDesc& desc) const;
	void destroyDescriptorPool(VkDescriptorPool pool) const;

	std::shared_ptr<VulkanDescriptorSet> createDescriptorSet(
		const VkDescriptorSetAllocateInfo& ai,
		std::shared_ptr<const VulkanDescriptorSetLayout> layout,
		std::shared_ptr<const VulkanDescriptorPool> pool) const;
	void destroyDescriptorSet(VkDescriptorPool pool, VkDescriptorSet set) const;

	void updateDescriptorSets(const std::vector<VkWriteDescriptorSet>& writes) const;

	std::shared_ptr<VulkanRenderPass> createRenderPass(const RenderPassDesc& desc) const;
	void destroyRenderPass(VkRenderPass handle) const;

	std::shared_ptr<VulkanFrameBuffer> createFrameBuffer(const VkFramebufferCreateInfo& ci) const;
	void destroyFrameBuffer(VkFramebuffer buffer) const;

	std::shared_ptr<VulkanGraphicsPipeline> createGraphicsPipeline(
		GraphicsPipelineDesc&& desc) const;
	void destroyPipeline(VkPipeline pipeline) const;

	std::shared_ptr<VulkanFence> createFence(const VkFenceCreateInfo& ci) const;
	void destroyFence(VkFence fence) const;

	std::shared_ptr<VulkanSemaphore> createSemaphore(const VkSemaphoreCreateInfo& ci) const;
	void destroySemaphore(VkSemaphore semaphore) const;

	uint32_t acquireNextImage(VkSwapchainKHR swapchain, VkSemaphore semaphore, VkFence fence) const;

	void waitIdle() const;

	void waitForFences(const std::vector<std::shared_ptr<VulkanFence>>& fences) const;
	void waitForFences(const std::vector<VkFence>& fenceHandles) const;

	void resetFences(const std::vector<std::shared_ptr<VulkanFence>>& fences) const;
	void resetFences(const std::vector<VkFence>& fenceHandles) const;

private:
	VulkanDevice(VkDevice device,
				 DeviceDesc desc,
				 std::shared_ptr<const VulkanPhysicalDevice> physical_device,
				 std::shared_ptr<const VulkanInstance> instance);

	VkPipelineLayout createPipelineLayout(const VkPipelineLayoutCreateInfo& ci) const;
	VkPipeline createPipeline(const VkGraphicsPipelineCreateInfo& ci) const;
	VkDevice device;
	DeviceDesc desc;
	std::shared_ptr<const VulkanPhysicalDevice> physicalDevice;
	std::shared_ptr<const VulkanInstance> instance;
	std::unordered_map<std::pair<int, int>, std::shared_ptr<VulkanQueue>, PairHasher<int, int>>
		queues;
};

}  // namespace xd

#endif	// XD_RT_VULKANDEVICE_H
