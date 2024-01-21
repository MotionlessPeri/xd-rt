//
// Created by Frank on 2024/1/18.
//

#ifndef XD_RT_VULKANDESCS_H
#define XD_RT_VULKANDESCS_H
#include <memory>
#include <vector>
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {
struct DeviceQueueDesc {
	VkDeviceQueueCreateInfo ci;
	std::vector<float> priorities;
};
struct QueueDesc {
	DeviceQueueDesc* deviceQueueDesc;
	uint32_t index;
};
struct DeviceDesc {
	VkDeviceCreateInfo ci;
	std::vector<DeviceQueueDesc> deviceQueueDescs;
	std::vector<const char*> enabledExtensionNames;
};
struct DescriptorSetLayoutDesc {
	VkDescriptorSetLayoutCreateInfo ci;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};
struct SubpassDesc {
	SubpassDesc() = default;
	SubpassDesc(const SubpassDesc& other) = default;
	SubpassDesc(SubpassDesc&& other) noexcept = default;
	SubpassDesc& operator=(const SubpassDesc& other) = default;
	SubpassDesc& operator=(SubpassDesc&& other) noexcept = default;
	VkSubpassDescription2 desc;
	std::vector<VkAttachmentReference2> inputAttaches;
	std::vector<VkAttachmentReference2> colorAttaches;
	std::vector<VkAttachmentReference2>
		resolveAttaches;  // resolve attaches must be in the same size of color attaches
	std::vector<VkAttachmentReference2> depthStencilAttaches;
	std::vector<uint32_t> preserveAttaches;
	uint32_t getAttachmentCount() const
	{
		return colorAttaches.size() + depthStencilAttaches.size();
	}
};
struct RenderPassDesc {
	VkRenderPassCreateInfo2 ci;
	std::vector<VkAttachmentDescription2> attachments;
	std::vector<SubpassDesc> subpasses;
	std::vector<VkSubpassDependency2> dependencys;
};
struct PipelineDescBase {
	VkPipelineLayoutCreateInfo layoutCi;
};
struct GraphicsPipelineDesc : public PipelineDescBase {
	VkGraphicsPipelineCreateInfo ci;
	std::vector<VkDynamicState> dynamicStates;
	std::vector<VkPipelineColorBlendAttachmentState> colorBlendStates;
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
};
struct SubmitInfoContainer {
	std::vector<std::shared_ptr<VulkanSemaphore>> waitingSemaphores;
	std::vector<VkPipelineStageFlags> waitingStages;
	std::vector<std::shared_ptr<VulkanSemaphore>> signalingSemaphores;
	std::shared_ptr<VulkanFence> signalingFence;
};
struct QueuePresentInfoContainer {
	std::vector<std::shared_ptr<VulkanSemaphore>> waitSemaphores;
	std::vector<std::shared_ptr<VulkanSwapchain>> swapchains;
	std::vector<uint32_t> imageIndices;
};
struct VertexInputStateDesc {
	VkPipelineVertexInputStateCreateInfo ci;
	std::vector<VkVertexInputBindingDescription> bindingDescs;
	std::vector<VkVertexInputAttributeDescription> attrDescs;
};
struct DescriptorPoolDesc {
	VkDescriptorPoolCreateInfo ci;
	std::vector<VkDescriptorPoolSize> poolSizes;
};
}  // namespace xd
#endif	// XD_RT_VULKANDESCS_H
