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
class VulkanPipelineBase;
class VulkanGraphicsPipeline;
class VulkanComputePipeline;
class VulkanDescriptorSetLayout;
class VulkanDescriptorPool;
class VulkanDescriptorSet;
class VulkanSubpass;
class VulkanRenderPass;
class VulkanFrameBuffer;
class VulkanFence;
class VulkanSemaphore;
class VulkanSampler;
class VulkanPipelineLayout;

class ModelFactoryVk;
class TriangleMeshVk;

class TextureFactoryVk;
class TextureVk;

class SamplerCache;

class FGBuilder;
class FGPass;
class FrameGraph;

class MaterialTemplateVk;
class MaterialInstanceVk;
class MaterialFactoryVk;

class LightManager;
}  // namespace xd
#endif	// XD_RT_VULKANTYPES_H