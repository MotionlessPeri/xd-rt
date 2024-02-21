//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_BASICCOMPUTEAPP_H
#define XD_RT_BASICCOMPUTEAPP_H
#include "../appBase/VulkanGLFWAppBase.h"
#include "backend/vulkan/VulkanSwapchain.h"
namespace xd {

class BasicComputeApp : public VulkanGLFWAppBase {
public:
	BasicComputeApp(int width, int height, const char* title);

protected:
	void initVulkan(const std::vector<const char*>& instanceEnabledExtensions) override;

	void loadAssets() override;
	void createResources() override;
	void buildPipeline() override;
	void bindResources(uint32_t swapchainImageIndex);
	void buildFrameBuffers() override;
	void recordCommandBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
							 uint32_t imageIndex) override;
	void updateResources(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) override;
	void draw() override;

private:
	std::shared_ptr<TextureVk> inputImage;
	std::vector<std::shared_ptr<TextureVk>> swapchainImages{};
	std::shared_ptr<MaterialTemplateVk> mtlTemplate;
	std::shared_ptr<MaterialInstanceVk> mtlInstance;

	struct FrameSync {
		std::shared_ptr<VulkanSemaphore> imageAcquireComplete;
		std::shared_ptr<VulkanSemaphore> renderComplete;
		std::shared_ptr<VulkanFence> submitFence;
	};
	std::vector<FrameSync> syncObjects{};

	std::shared_ptr<VulkanCommandPool> computeCmdPool = nullptr;
	std::vector<std::shared_ptr<VulkanCommandBuffer>> computeCmdBuffers{};
	struct {
		uint32_t width;
		uint32_t height;
	} imageInfo;
	std::shared_ptr<VulkanBuffer> imageInfoBuffer = nullptr;
};

}  // namespace xd

#endif	// XD_RT_VULKANGLFWWINDOW_H
