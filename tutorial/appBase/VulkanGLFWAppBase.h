//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_VULKANGLFWAPPBASE_H
#define XD_RT_VULKANGLFWAPPBASE_H
#include <memory>
#include <vector>
#include "GLFWInclude.h"
#include "backend/vulkan/VulkanTypes.h"

namespace xd {

class VulkanGLFWAppBase {
public:
	VulkanGLFWAppBase(int width, int height, const char* title);
	void init();
	void run();

protected:
	virtual GLFWwindow* createWindow(int width,
									 int height,
									 const char* title,
									 std::vector<const char*>& instanceEnableExts);
	virtual void initVulkan(const std::vector<const char*>& instanceEnabledExtensions);
	virtual void handleInput(GLFWwindow* window);
	virtual void loadAssets() = 0;
	virtual void createResources() = 0;
	virtual void buildRenderPass() = 0;
	virtual void bindResources() = 0;
	virtual void buildFrameBuffers() = 0;
	virtual void recordCommandBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
									 uint32_t imageIndex) = 0;
	virtual void updateResources(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) = 0;
	virtual void draw() = 0;
	int width, height;
	const char* title;
	GLFWwindow* window = nullptr;
	std::shared_ptr<VulkanInstance> instance = nullptr;
	std::shared_ptr<VulkanPhysicalDevice> physicalDevice = nullptr;
	std::shared_ptr<VulkanDevice> device = nullptr;
	std::shared_ptr<VulkanQueue> presentQueue = nullptr;
	std::shared_ptr<VulkanQueue> graphicQueue = nullptr;
	std::shared_ptr<VulkanQueue> computeQueue = nullptr;
	uint32_t currentFrame = 0u;
	uint32_t frameCount;
	std::shared_ptr<VulkanSwapchain> swapchain = nullptr;
	float elapsedTime = 0.f;
};

}  // namespace xd

#endif	// XD_RT_VULKANGLFWWINDOW_H
