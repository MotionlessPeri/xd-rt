//
// Created by Frank on 2024/2/23.
//

#ifndef XD_RT_BASICFRAMEGRAPHAPP_H
#define XD_RT_BASICFRAMEGRAPHAPP_H
#include <glm/glm.hpp>

#include "../imguiAppBase/ImguiAppBase.h"
namespace xd {

class BasicFramegraphApp : public ImguiAppBase {
public:
	BasicFramegraphApp(int width, int height, const char* title);

protected:
	void loadAssets() override;
	void createResources() override;
	void buildPipeline() override;
	void buildFrameBuffers() override;
	void recordCommandBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
							 uint32_t imageIndex) override;
	void updateResources(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) override;
	void render() override;
	void renderImgui() override;
	void bindGraphicPipelineResources();
	void bindComputePipelineResources();
	void bindResources();

	void recordGraphicCmdBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer);
	void recordComputeCmdBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer);
	void present() override;

private:
	std::vector<std::shared_ptr<TextureVk>> swapchainImages{};
	struct FrameBufferResource {
		std::shared_ptr<VulkanImage> colorAttach;
		std::shared_ptr<VulkanImageView> colorAttachView;
		std::shared_ptr<VulkanImage> depthStencilAttach;
		std::shared_ptr<VulkanImageView> depthStencilView;
	};
	std::vector<FrameBufferResource> frameBufferResources{};
	std::vector<std::shared_ptr<VulkanFrameBuffer>> graphicFrameBuffers;
	std::shared_ptr<VulkanRenderPass> graphicRenderPass = nullptr;
	struct Material {
		std::shared_ptr<MaterialTemplateVk> mtlTemplate = nullptr;
		std::shared_ptr<MaterialInstanceVk> mtlInstance = nullptr;
	} lambertianMtl, tonemappingMtl;
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
		uint32_t width;
		uint32_t height;
		uint32_t enableTonemapping;
	} imageInfo;
	std::shared_ptr<VulkanBuffer> imageInfoBuffer = nullptr;
	struct {
		std::shared_ptr<TextureVk> diffuse = nullptr;
		std::shared_ptr<TextureVk> normal = nullptr;
	} mtlResources;
	std::shared_ptr<LightManager> lightManager = nullptr;

	std::vector<FrameSync> graphicSyncObjects{};
	std::vector<FrameSync> computeSyncObjects{};

	bool enableTonemapping = true;
};

}  // namespace xd

#endif	// XD_RT_BASICFRAMEGRAPHAPP_H
