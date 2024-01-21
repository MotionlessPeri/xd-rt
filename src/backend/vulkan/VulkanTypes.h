//
// Created by Frank on 2024/1/9.
//

#ifndef XD_RT_VULKANTYPES_H
#define XD_RT_VULKANTYPES_H
namespace xd {
class VulkanInstance;
class VulkanPhysicalDevice;
class VulkanDevice;
template <typename ObjDescType>
class VulkanDeviceObject;
class VulkanQueue;
class VulkanImage;
class VulkanImageView;
class VulkanSurface;
class VulkanSwapchain;
class VulkanShader;
class VulkanBuffer;
class VulkanMemory;
class VulkanCommandPool;
class VulkanCommandBuffer;
class VulkanPipelineLayout;
class VulkanGraphicsPipeline;
class VulkanDescriptorSetLayout;
class VulkanDescriptorPool;
class VulkanDescriptorSet;
class VulkanSubpass;
class VulkanRenderPass;
class VulkanFrameBuffer;
class VulkanFence;
class VulkanSemaphore;
class MaterialTemplateVk;
class MaterialInstanceVk;
class TriangleMeshVk;

class FrameGraphBuilder;
class FrameGraphPass;
class FrameGraph;
}  // namespace xd
#endif	// XD_RT_VULKANTYPES_H
