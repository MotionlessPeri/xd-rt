//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_VULKANGLFWWINDOW_H
#define XD_RT_VULKANGLFWWINDOW_H
#include <memory>
#include <vector>
#include "GLFWInclude.h"
#include "backend/vulkan/VulkanTypes.h"
namespace xd {

class VulkanGLFWApp {
public:
	VulkanGLFWApp(int width, int height, const char* title);
	void run();

private:
	void loadAssets();
	void buildRenderPass();
	void buildPipeline();
	void buildFrameBuffers();
	void recordCommandBuffer(uint32_t imageIndex);
	void draw();
	GLFWwindow* window = nullptr;
	std::shared_ptr<VulkanInstance> instance = nullptr;
	std::shared_ptr<VulkanPhysicalDevice> physicalDevice = nullptr;
	std::shared_ptr<VulkanDevice> device = nullptr;
	std::shared_ptr<VulkanQueue> presentQueue = nullptr;
	uint32_t currentFrame = 0u;
	uint32_t frameCount;
	std::shared_ptr<VulkanCommandPool> pool = nullptr;
	std::vector<std::shared_ptr<VulkanCommandBuffer>> cmdBuffers{};
	struct FrameSync {
		std::shared_ptr<VulkanSemaphore> imageAcquireComplete;
		std::shared_ptr<VulkanSemaphore> renderComplete;
		std::shared_ptr<VulkanFence> submitFence;
	};
	std::vector<FrameSync> syncObjects{};
	std::shared_ptr<VulkanSwapchain> swapchain = nullptr;
	std::vector<std::shared_ptr<VulkanShader>> shaders{};
	std::shared_ptr<VulkanRenderPass> renderPass = nullptr;
	std::shared_ptr<VulkanGraphicsPipeline> pipeline = nullptr;
	std::vector<std::shared_ptr<VulkanFrameBuffer>> frameBuffers{};

	std::shared_ptr<TriangleMeshVk> model;
};

}  // namespace xd

#endif	// XD_RT_VULKANGLFWWINDOW_H
