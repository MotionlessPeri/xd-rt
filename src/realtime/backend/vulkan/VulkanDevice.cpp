//
// Created by Frank on 2024/1/9.
//

#include "VulkanDevice.h"

#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanComputePipeline.h"
#include "VulkanDescriptorPool.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanFence.h"
#include "VulkanFrameBuffer.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanInstance.h"
#include "VulkanMacros.h"
#include "VulkanMemory.h"
#include "VulkanPipelineLayout.h"
#include "VulkanQueue.h"
#include "VulkanRenderPass.h"
#include "VulkanSampler.h"
#include "VulkanSemaphore.h"
#include "VulkanShader.h"
#include "VulkanSurface.h"
#include "VulkanSwapchain.h"
using namespace xd;
VulkanDevice::VulkanDevice(VkDevice device,
						   DeviceDesc desc,
						   std::shared_ptr<const VulkanPhysicalDevice> physical_device,
						   std::shared_ptr<const VulkanInstance> instance)
	: device(device),
	  desc(std::move(desc)),
	  physicalDevice(std::move(physical_device)),
	  instance(std::move(instance))
{
}

VulkanDevice::~VulkanDevice()
{
	vkDestroyDevice(device, nullptr);
}

std::shared_ptr<VulkanQueue> VulkanDevice::getQueue(int queueFamilyIndex, int queueIndex)
{
	return getQueue({queueFamilyIndex, queueIndex});
}

std::shared_ptr<VulkanQueue> VulkanDevice::getQueue(const std::pair<int, int>& key)
{
	const auto it = queues.find(key);
	if (it != queues.end())
		return it->second;
	VkQueue queue;
	vkGetDeviceQueue(device, key.first, key.second, &queue);
	QueueDesc queueDesc;
	queueDesc.deviceQueueDesc = &desc.deviceQueueDescs[key.first];
	queueDesc.index = 0u;
	auto res = std::shared_ptr<VulkanQueue>{new VulkanQueue{shared_from_this(), queueDesc, queue}};
	queues[key] = res;
	return res;
}

std::shared_ptr<VulkanSurface> VulkanDevice::createSurface(const SurfaceCIType& ci) const
{
	VkSurfaceKHR surfaceHandle;
	CHECK_VK_ERROR(vkCreateWin32SurfaceKHR(instance->instance, &ci, nullptr, &surfaceHandle));
	return std::shared_ptr<VulkanSurface>{
		new VulkanSurface{shared_from_this(), ci, instance, physicalDevice, surfaceHandle}};
}

std::shared_ptr<VulkanSurface> VulkanDevice::createSurface(const SurfaceCIType& ci,
														   VkSurfaceKHR surface) const
{
	return std::shared_ptr<VulkanSurface>{
		new VulkanSurface{shared_from_this(), ci, instance, physicalDevice, surface}};
}

void VulkanDevice::destroySurface(VkSurfaceKHR surface) const
{
	vkDestroySurfaceKHR(instance->instance, surface, nullptr);
}

std::shared_ptr<VulkanSwapchain> VulkanDevice::createSwapchain(const VkSwapchainCreateInfoKHR& ci,
															   VkExtent2D extent,
															   VkSurfaceFormatKHR format) const
{
	VkSwapchainKHR swapchainHandle;
	CHECK_VK_ERROR(vkCreateSwapchainKHR(device, &ci, nullptr, &swapchainHandle));
	return std::shared_ptr<VulkanSwapchain>{
		new VulkanSwapchain{shared_from_this(), ci, swapchainHandle, extent, format}};
}

void VulkanDevice::destroySwapchain(VkSwapchainKHR swapchainHandle) const
{
	vkDestroySwapchainKHR(device, swapchainHandle, nullptr);
}

std::vector<std::shared_ptr<VulkanImage>> VulkanDevice::getSwapchainImages(
	VkSwapchainKHR swapchain) const
{
	uint32_t imageCount;
	CHECK_VK_ERROR(vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));
	std::vector<VkImage> swapchainImageHandles{imageCount};
	CHECK_VK_ERROR(
		vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImageHandles.data()));
	const auto transformedView =
		swapchainImageHandles | std::views::transform([&](VkImage image) {
			return std::shared_ptr<VulkanImage>{new VulkanImage{shared_from_this(), image}};
		});
	return {transformedView.begin(), transformedView.end()};
}

std::shared_ptr<VulkanImage> VulkanDevice::createImage(const VkImageCreateInfo& ci,
													   VkMemoryPropertyFlags properties) const
{
	VkImage handle;
	vkCreateImage(device, &ci, nullptr, &handle);
	return std::shared_ptr<VulkanImage>{
		new VulkanImage{shared_from_this(), ci, handle, properties}};
}

void VulkanDevice::destroyImage(VkImage image) const
{
	vkDestroyImage(device, image, nullptr);
}

std::shared_ptr<VulkanImageView> VulkanDevice::createImageView(
	std::shared_ptr<const VulkanImage> image,
	const VkImageViewCreateInfo& ci) const
{
	VkImageView viewHandle;
	CHECK_VK_ERROR(vkCreateImageView(device, &ci, nullptr, &viewHandle));
	return std::shared_ptr<VulkanImageView>{
		new VulkanImageView{shared_from_this(), std::move(ci), image, viewHandle}};
}

void VulkanDevice::destroyImageView(VkImageView imageView) const
{
	vkDestroyImageView(device, imageView, nullptr);
}

std::shared_ptr<VulkanShader> VulkanDevice::createShader(const VkShaderModuleCreateInfo& ci,
														 VkShaderStageFlagBits stage,
														 std::string entry_point_name) const
{
	VkShaderModule handle;
	CHECK_VK_ERROR(vkCreateShaderModule(device, &ci, nullptr, &handle));
	return std::shared_ptr<VulkanShader>{
		new VulkanShader{shared_from_this(), ci, handle, stage, std::move(entry_point_name)}};
}

void VulkanDevice::destroyShader(VkShaderModule shaderModuleHandle) const
{
	vkDestroyShaderModule(device, shaderModuleHandle, nullptr);
}

std::shared_ptr<VulkanCommandPool> VulkanDevice::createCommandPool(
	VkCommandPoolCreateInfo&& ci) const
{
	VkCommandPool pool;
	CHECK_VK_ERROR(vkCreateCommandPool(device, &ci, nullptr, &pool));
	// TODO: now we always returns queueIndex 0 for the queue family, change it when necessary
	const auto queue = queues.at({ci.queueFamilyIndex, 0});
	return std::shared_ptr<VulkanCommandPool>{
		new VulkanCommandPool{shared_from_this(), ci, queue, pool}};
}

void VulkanDevice::destroyCommandPool(VkCommandPool pool) const
{
	vkDestroyCommandPool(device, pool, nullptr);
}

std::vector<std::shared_ptr<VulkanCommandBuffer>> VulkanDevice::createCommandBuffers(
	const VkCommandBufferAllocateInfo& ai,
	const std::shared_ptr<const VulkanCommandPool>& poolRef) const
{
	std::vector<VkCommandBuffer> handles(ai.commandBufferCount);
	CHECK_VK_ERROR(vkAllocateCommandBuffers(device, &ai, handles.data()));
	std::vector<std::shared_ptr<VulkanCommandBuffer>> ret(ai.commandBufferCount);
	std::ranges::transform(
		handles, ret.begin(), [&](VkCommandBuffer handle) -> std::shared_ptr<VulkanCommandBuffer> {
			return std::shared_ptr<VulkanCommandBuffer>{
				new VulkanCommandBuffer{shared_from_this(), ai, poolRef, handle}};
		});
	return ret;
}

void VulkanDevice::freeCommandBuffer(VkCommandPool pool, VkCommandBuffer cmdBuffer) const
{
	vkFreeCommandBuffers(device, pool, 1, &cmdBuffer);
}

void VulkanDevice::freeCommandBuffers(VkCommandPool pool,
									  const std::vector<VkCommandBuffer>& cmdBuffers) const
{
	vkFreeCommandBuffers(device, pool, cmdBuffers.size(), cmdBuffers.data());
}

std::shared_ptr<VulkanBuffer> VulkanDevice::createBuffer(const VkBufferCreateInfo& ci,
														 VkMemoryPropertyFlags properties) const
{
	VkBuffer handle;
	CHECK_VK_ERROR(vkCreateBuffer(device, &ci, nullptr, &handle));
	return std::shared_ptr<VulkanBuffer>{
		new VulkanBuffer{shared_from_this(), ci, handle, properties}};
}

VkMemoryRequirements VulkanDevice::getBufferMemoryRequirements(VkBuffer buffer) const
{
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);
	return memRequirements;
}

VkMemoryRequirements VulkanDevice::getImageMemoryRequirements(VkImage image) const
{
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);
	return memRequirements;
}

void VulkanDevice::bindBufferMemory(VkBuffer buffer,
									VkDeviceMemory memory,
									VkDeviceSize memoryOffset) const
{
	CHECK_VK_ERROR(vkBindBufferMemory(device, buffer, memory, memoryOffset));
}

void VulkanDevice::bindImageMemory(VkImage image,
								   VkDeviceMemory memory,
								   VkDeviceSize memoryOffset) const
{
	CHECK_VK_ERROR(vkBindImageMemory(device, image, memory, memoryOffset));
}

void VulkanDevice::destroyBuffer(VkBuffer bufferHandle) const
{
	vkDestroyBuffer(device, bufferHandle, nullptr);
}

const VkMemoryType& VulkanDevice::getMemoryType(uint32_t index) const
{
	const auto& memProperties = physicalDevice->getMemoryProperties();
	if (index >= memProperties.memoryTypeCount)
		throw std::runtime_error{"Invalid memoryTypeIndex.\n"};
	return memProperties.memoryTypes[index];
}

std::unique_ptr<VulkanMemory> VulkanDevice::allocateMemory(
	VkMemoryAllocateInfo&& ai,
	const VkMemoryRequirements& memRequirements,
	VkMemoryPropertyFlags properties) const
{
	ai.memoryTypeIndex =
		physicalDevice->getProperMemoryTypeIndex(memRequirements.memoryTypeBits, properties);
	VkDeviceMemory memory;
	CHECK_VK_ERROR(vkAllocateMemory(device, &ai, nullptr, &memory));
	return std::unique_ptr<VulkanMemory>{new VulkanMemory{shared_from_this(), ai, memory}};
}

void VulkanDevice::freeMemory(VkDeviceMemory memory) const
{
	vkFreeMemory(device, memory, nullptr);
}

void* VulkanDevice::mapMemory(VkDeviceMemory memoryHandle, uint32_t offset, uint32_t size) const
{
	void* ret;
	CHECK_VK_ERROR(vkMapMemory(device, memoryHandle, offset, size, 0, &ret));
	return ret;
}

void VulkanDevice::unmapMemory(VkDeviceMemory memoryHandle) const
{
	vkUnmapMemory(device, memoryHandle);
}

void VulkanDevice::flushMemory(const std::vector<VkMappedMemoryRange>& ranges) const
{
	CHECK_VK_ERROR(vkFlushMappedMemoryRanges(device, ranges.size(), ranges.data()));
}

std::shared_ptr<VulkanDescriptorSetLayout> VulkanDevice::createDescriptorSetLayout(
	const DescriptorSetLayoutDesc& desc,
	std::unordered_map<std::string, uint32_t> nameToBinding) const
{
	VkDescriptorSetLayout layoutHandle;
	CHECK_VK_ERROR(vkCreateDescriptorSetLayout(device, &desc.ci, nullptr, &layoutHandle));
	return std::shared_ptr<VulkanDescriptorSetLayout>{new VulkanDescriptorSetLayout{
		shared_from_this(), desc, layoutHandle, std::move(nameToBinding)}};
}

void VulkanDevice::destroyDescriptorSetLayout(VkDescriptorSetLayout layout) const
{
	vkDestroyDescriptorSetLayout(device, layout, nullptr);
}

std::shared_ptr<VulkanDescriptorPool> VulkanDevice::createDescriptorPool(
	const DescriptorPoolDesc& desc) const
{
	VkDescriptorPool pool;
	CHECK_VK_ERROR(vkCreateDescriptorPool(device, &desc.ci, nullptr, &pool));
	return std::shared_ptr<VulkanDescriptorPool>{
		new VulkanDescriptorPool{shared_from_this(), desc, pool}};
}

void VulkanDevice::destroyDescriptorPool(VkDescriptorPool pool) const
{
	vkDestroyDescriptorPool(device, pool, nullptr);
}

std::shared_ptr<VulkanDescriptorSet> VulkanDevice::createDescriptorSet(
	const VkDescriptorSetAllocateInfo& ai,
	std::shared_ptr<const VulkanDescriptorSetLayout> layout,
	std::shared_ptr<const VulkanDescriptorPool> pool) const
{
	VkDescriptorSet set;
	const auto res = vkAllocateDescriptorSets(device, &ai, &set);
	CHECK_VK_ERROR(res);
	return std::shared_ptr<VulkanDescriptorSet>{
		new VulkanDescriptorSet{shared_from_this(), ai, std::move(layout), std::move(pool), set}};
}

void VulkanDevice::destroyDescriptorSet(VkDescriptorPool pool, VkDescriptorSet set) const
{
	vkFreeDescriptorSets(device, pool, 1, &set);
}

void VulkanDevice::updateDescriptorSets(const std::vector<VkWriteDescriptorSet>& writes) const
{
	vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);
}

std::shared_ptr<VulkanRenderPass> VulkanDevice::createRenderPass(const RenderPassDesc& desc) const
{
	VkRenderPass handle;
	CHECK_VK_ERROR(vkCreateRenderPass2(device, &desc.ci, nullptr, &handle));
	return std::shared_ptr<VulkanRenderPass>{
		new VulkanRenderPass{shared_from_this(), desc, handle}};
}

void VulkanDevice::destroyRenderPass(VkRenderPass handle) const
{
	vkDestroyRenderPass(device, handle, nullptr);
}

std::shared_ptr<VulkanFrameBuffer> VulkanDevice::createFrameBuffer(
	const VkFramebufferCreateInfo& ci) const
{
	VkFramebuffer handle;
	CHECK_VK_ERROR(vkCreateFramebuffer(device, &ci, nullptr, &handle));
	return std::shared_ptr<VulkanFrameBuffer>{
		new VulkanFrameBuffer{shared_from_this(), ci, handle}};
}

void VulkanDevice::destroyFrameBuffer(VkFramebuffer buffer) const
{
	vkDestroyFramebuffer(device, buffer, nullptr);
}

std::shared_ptr<VulkanPipelineLayout> VulkanDevice::createPipelineLayout(
	PipelineLayoutDesc&& desc) const
{
	const auto layoutsView = desc.setLayouts | std::views::transform([](const auto& setLayoutPtr) {
								 return setLayoutPtr->layout;
							 });
	const std::vector<VkDescriptorSetLayout> layouts{layoutsView.begin(), layoutsView.end()};
	desc.ci.setLayoutCount = layouts.size();
	desc.ci.pSetLayouts = layouts.empty() ? nullptr : layouts.data();
	desc.ci.pushConstantRangeCount = desc.pushConstants.size();
	desc.ci.pPushConstantRanges = desc.pushConstants.empty() ? nullptr : desc.pushConstants.data();
	VkPipelineLayout handle;
	vkCreatePipelineLayout(device, &desc.ci, nullptr, &handle);
	return std::shared_ptr<VulkanPipelineLayout>{
		new VulkanPipelineLayout{shared_from_this(), desc, handle}};
}

void VulkanDevice::destroyPipelineLayout(VkPipelineLayout layout) const
{
	vkDestroyPipelineLayout(device, layout, nullptr);
}

std::shared_ptr<VulkanGraphicsPipeline> VulkanDevice::createGraphicsPipeline(
	GraphicsPipelineDesc&& desc,
	std::shared_ptr<VulkanPipelineLayout> layout) const
{
	desc.ci.layout = layout->handle;
	VkPipeline handle;
	vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &desc.ci, nullptr, &handle);
	return std::shared_ptr<VulkanGraphicsPipeline>{
		new VulkanGraphicsPipeline{shared_from_this(), desc, handle, layout}};
}

std::shared_ptr<VulkanComputePipeline> VulkanDevice::createComputePipeline(
	VkComputePipelineCreateInfo&& ci,
	std::shared_ptr<VulkanPipelineLayout> layout) const
{
	ci.layout = layout->handle;
	VkPipeline handle;
	vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &ci, nullptr, &handle);
	return std::shared_ptr<VulkanComputePipeline>{
		new VulkanComputePipeline{shared_from_this(), ci, handle, layout}};
}

void VulkanDevice::destroyPipeline(VkPipeline pipeline) const
{
	vkDestroyPipeline(device, pipeline, nullptr);
}

std::shared_ptr<VulkanFence> VulkanDevice::createFence(const VkFenceCreateInfo& ci) const
{
	VkFence fence;
	CHECK_VK_ERROR(vkCreateFence(device, &ci, nullptr, &fence));
	return std::shared_ptr<VulkanFence>{new VulkanFence{shared_from_this(), ci, fence}};
}
void VulkanDevice::destroyFence(VkFence fence) const
{
	vkDestroyFence(device, fence, nullptr);
}

std::shared_ptr<VulkanSemaphore> VulkanDevice::createSemaphore(
	const VkSemaphoreCreateInfo& ci) const
{
	VkSemaphore semaphore;
	CHECK_VK_ERROR(vkCreateSemaphore(device, &ci, nullptr, &semaphore));
	return std::shared_ptr<VulkanSemaphore>{new VulkanSemaphore{shared_from_this(), ci, semaphore}};
}

void VulkanDevice::destroySemaphore(VkSemaphore semaphore) const
{
	vkDestroySemaphore(device, semaphore, nullptr);
}

uint32_t VulkanDevice::acquireNextImage(VkSwapchainKHR swapchain,
										VkSemaphore semaphore,
										VkFence fence) const
{
	uint32_t index;
	CHECK_VK_ERROR(vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore, fence, &index));
	return index;
}

void VulkanDevice::waitIdle() const
{
	vkDeviceWaitIdle(device);
}

void VulkanDevice::waitForFences(const std::vector<std::shared_ptr<VulkanFence>>& fences) const
{
	const auto handlesView =
		fences | std::views::transform([](const auto& fence) { return fence->fence; });
	waitForFences(std::vector<VkFence>{handlesView.begin(), handlesView.end()});
}

void VulkanDevice::waitForFences(const std::vector<VkFence>& fenceHandles) const
{
	CHECK_VK_ERROR(
		vkWaitForFences(device, fenceHandles.size(), fenceHandles.data(), VK_TRUE, UINT64_MAX));
}

void VulkanDevice::resetFences(const std::vector<std::shared_ptr<VulkanFence>>& fences) const
{
	const auto handlesView =
		fences | std::views::transform([](const auto& fence) { return fence->fence; });
	resetFences(std::vector<VkFence>{handlesView.begin(), handlesView.end()});
}

void VulkanDevice::resetFences(const std::vector<VkFence>& fenceHandles) const
{
	CHECK_VK_ERROR(vkResetFences(device, fenceHandles.size(), fenceHandles.data()));
}

std::shared_ptr<VulkanSampler> VulkanDevice::createSampler(const SamplerDesc& desc) const
{
	VkSampler handle;
	vkCreateSampler(device, &desc, nullptr, &handle);
	return std::shared_ptr<VulkanSampler>{new VulkanSampler{shared_from_this(), desc, handle}};
}

void VulkanDevice::destroySampler(VkSampler sampler) const
{
	vkDestroySampler(device, sampler, nullptr);
}
