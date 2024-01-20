//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_VULKANGLFWWINDOW_H
#define XD_RT_VULKANGLFWWINDOW_H
#include <memory>
#include <vector>
#include "GLFWInclude.h"
#include "backend/vulkan/VulkanTypes.h"
#include "glm/glm.hpp"
namespace xd {

class VulkanGLFWApp {
public:
	VulkanGLFWApp(int width, int height, const char* title);
	void run();

private:
	void handleInput(GLFWwindow* window);
	void loadAssets();
	void createResources();
	void buildRenderPass();
	void buildDescriptors();
	void bindResources();
	void buildPipeline();
	void buildFrameBuffers();
	void recordCommandBuffer(uint32_t imageIndex);
	void updateResources();
	void draw();
	int width, height;
	GLFWwindow* window = nullptr;
	std::shared_ptr<VulkanInstance> instance = nullptr;
	std::shared_ptr<VulkanPhysicalDevice> physicalDevice = nullptr;
	std::shared_ptr<VulkanDevice> device = nullptr;
	std::shared_ptr<VulkanQueue> presentQueue = nullptr;
	uint32_t currentFrame = 0u;
	uint32_t frameCount;
	std::shared_ptr<VulkanCommandPool> cmdPool = nullptr;
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
	std::shared_ptr<VulkanDescriptorSetLayout> descSetLayout = nullptr;
	std::shared_ptr<VulkanDescriptorPool> descPool = nullptr;
	struct FrameResource {
		std::shared_ptr<VulkanDescriptorSet> descSet = nullptr;
	};
	std::vector<FrameResource> frameResources;
	std::vector<std::shared_ptr<VulkanFrameBuffer>> frameBuffers{};

	std::shared_ptr<TriangleMeshVk> model;
	struct {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 normalTransform;	// we use mat4 instead of mat3 because of alignment requirements
	} uniformData;
	std::shared_ptr<VulkanBuffer> uniformBuffer = nullptr;
	float elapsedTime = 0.f;
};

}  // namespace xd

#endif	// XD_RT_VULKANGLFWWINDOW_H
