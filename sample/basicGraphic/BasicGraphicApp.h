//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_BASICCOMPUTEAPP_H
#define XD_RT_VULKANGLFWAPP_H
#include <memory>
#include <vector>
#include "../appBase/VulkanGLFWAppBase.h"
#include "backend/vulkan/FrameGraph.h"
#include "backend/vulkan/LightManager.h"
#include "backend/vulkan/VulkanTypes.h"
#include "glm/glm.hpp"
namespace xd {

class BasicGraphicApp : public VulkanGLFWAppBase {
public:
	BasicGraphicApp(int width, int height, const char* title);

private:
	void initVulkan(const std::vector<const char*>& instanceEnabledExtensions) override;

	void handleInput(GLFWwindow* window) override;
	void loadAssets() override;
	void createResources() override;
	void buildPipeline() override;
	void buildMaterial();
	void bindResources();
	void buildFrameBuffers() override;
	void recordCommandBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
							 uint32_t imageIndex) override;
	void updateResources(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) override;
	void render() override;

	std::shared_ptr<VulkanCommandPool> graphicCmdPool = nullptr;
	std::vector<std::shared_ptr<VulkanCommandBuffer>> graphicCmdBuffers{};
	struct FrameSync {
		std::shared_ptr<VulkanSemaphore> imageAcquireComplete;
		std::shared_ptr<VulkanSemaphore> renderComplete;
		std::shared_ptr<VulkanFence> submitFence;
	};
	std::vector<FrameSync> syncObjects{};
	std::vector<std::shared_ptr<VulkanShader>> shaders{};
	std::shared_ptr<FrameGraph> frameGraph = nullptr;
	struct FrameGraphHandles {
		FrameGraphResourceHandle color;
		FrameGraphResourceHandle depth;
	} fgHandles;
	struct FrameBufferResource {
		std::shared_ptr<VulkanImage> colorAttach;
		std::shared_ptr<VulkanImageView> colorAttachView;
		std::shared_ptr<VulkanImage> depthStencilAttach;
		std::shared_ptr<VulkanImageView> depthStencilView;
	};
	std::vector<FrameBufferResource> frameBufferResources{};
	std::vector<std::shared_ptr<VulkanFrameBuffer>> frameBuffers{};

	std::shared_ptr<TriangleMeshVk> model;
	struct {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 normalTransform;	// we use mat4 instead of mat3 because of alignment requirements
		glm::vec4 cameraWorldPos;
	} uniformData;
	std::shared_ptr<VulkanBuffer> uniformBuffer = nullptr;
	struct {
		std::shared_ptr<TextureVk> diffuse;
		std::shared_ptr<TextureVk> normal;
	} mtlResources;
	std::shared_ptr<MaterialTemplateVk> mtlTemplate = nullptr;
	std::shared_ptr<MaterialInstanceVk> mtlInstance = nullptr;
	std::shared_ptr<LightManager> lightManager = nullptr;
};

}  // namespace xd

#endif	// XD_RT_VULKANGLFWWINDOW_H
