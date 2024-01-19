//
// Created by Frank on 2024/1/13.
//

#ifndef XD_RT_VULKANPIPELINE_H
#define XD_RT_VULKANPIPELINE_H
#include <vector>
#include "VulkanDescs.h"
#include "VulkanDeviceObject.h"

namespace xd {

class VulkanPipelineBase {
public:
	VulkanPipelineBase(VkPipeline pipeline) : pipeline(pipeline) {}
	VulkanPipelineBase(const VulkanPipelineBase& other) = delete;
	VulkanPipelineBase(VulkanPipelineBase&& other) noexcept = delete;
	VulkanPipelineBase& operator=(const VulkanPipelineBase& other) = delete;
	VulkanPipelineBase& operator=(VulkanPipelineBase&& other) noexcept = delete;

protected:
	VkPipeline pipeline = VK_NULL_HANDLE;
};

class VulkanGraphicsPipeline : public VulkanDeviceObject<GraphicsPipelineDesc>,
							   public VulkanPipelineBase {
public:
	friend class VulkanDevice;
	friend class VulkanGLFWApp;	 // TODO: remove it ASAP when FrameGraph is built
	VulkanGraphicsPipeline() = delete;
	VulkanGraphicsPipeline(const VulkanGraphicsPipeline& other) = delete;
	VulkanGraphicsPipeline(VulkanGraphicsPipeline&& other) noexcept = delete;
	VulkanGraphicsPipeline& operator=(const VulkanGraphicsPipeline& other) = delete;
	VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline&& other) noexcept = delete;
	~VulkanGraphicsPipeline();

private:
	VulkanGraphicsPipeline(std::shared_ptr<const VulkanDevice> device,
						   const GraphicsPipelineDesc& desc,
						   VkPipeline pipeline);
};

}  // namespace xd

#endif	// XD_RT_VULKANPIPELINE_H
