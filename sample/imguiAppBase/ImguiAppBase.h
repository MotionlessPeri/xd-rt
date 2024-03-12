//
// Created by Frank on 2024/2/22.
//

#ifndef XD_RT_IMGUIAPPBASE_H
#define XD_RT_IMGUIAPPBASE_H

#include "../appBase/VulkanGLFWAppBase.h"
#include "backend/vulkan/VulkanDescriptorPool.h"
namespace xd {
class ImguiAppBase : public VulkanGLFWAppBase {
public:
	void init() override;
	void run() override;

	ImguiAppBase(int width, int height, const char* title);

protected:
	void initVulkan(const std::vector<const char*>& instanceEnabledExtensions) override;
	void initImgui();
	virtual void renderImgui();
	void recordImguiCmdBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer);
	virtual void present();
	struct FrameSync {
		std::shared_ptr<VulkanSemaphore> imageAcquireComplete;
		std::shared_ptr<VulkanSemaphore> renderComplete;
		std::shared_ptr<VulkanFence> submitFence;
	};
	std::vector<FrameSync> imguiSyncObjects{};
	std::shared_ptr<VulkanCommandPool> graphicCmdPool = nullptr;
	std::vector<std::shared_ptr<VulkanCommandBuffer>> graphicCmdBuffers{};
	std::shared_ptr<VulkanCommandPool> computeCmdPool = nullptr;
	std::vector<std::shared_ptr<VulkanCommandBuffer>> computeCmdBuffers{};
	std::shared_ptr<VulkanDescriptorPool> descPool = nullptr;
	std::shared_ptr<VulkanCommandPool> imguiCmdPool = nullptr;
	std::vector<std::shared_ptr<VulkanCommandBuffer>> imguiCmdBuffers{};
	std::vector<std::shared_ptr<VulkanFrameBuffer>> imguiFrameBuffers{};
	std::shared_ptr<VulkanRenderPass> imguiRenderPass;
};
}  // namespace xd

#endif	// XD_RT_IMGUIAPPBASE_H
