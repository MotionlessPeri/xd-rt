//
// Created by Frank on 2024/2/22.
//

#ifndef XD_RT_TONEMAPPINGAPP_H
#define XD_RT_TONEMAPPINGAPP_H
#include "../imguiAppBase/ImguiAppBase.h"
namespace xd {

class TonemappingApp : public ImguiAppBase {
public:
	TonemappingApp(int width, int height, const char* title);

protected:
	void initVulkan(const std::vector<const char*>& instanceEnabledExtensions) override;
	void loadAssets() override;
	void createResources() override;
	void buildPipeline() override;
	void buildFrameBuffers() override;
	void recordCommandBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
							 uint32_t imageIndex) override;
	void updateResources(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) override;
	void bindResources(uint32_t swapchainImageIndex);
	void render() override;
	void renderImgui() override;
	void present() override;
	struct ComputeSyncObject {
		std::shared_ptr<VulkanSemaphore> computeComplete;
		std::shared_ptr<VulkanSemaphore> imageAcquireComplete;
		std::shared_ptr<VulkanFence> submitFence;
	};
	std::vector<ComputeSyncObject> computeSyncObjects{};
	std::shared_ptr<TextureVk> inputImage;
	std::vector<std::shared_ptr<TextureVk>> swapchainImages{};
	std::shared_ptr<MaterialTemplateVk> mtlTemplate;
	std::shared_ptr<MaterialInstanceVk> mtlInstance;

	struct {
		uint32_t width;
		uint32_t height;
		uint32_t enableTonemapping;
	} imageInfo;
	bool enableTonemapping;
	std::shared_ptr<VulkanBuffer> imageInfoBuffer = nullptr;
};

}  // namespace xd

#endif	// XD_RT_TONEMAPPINGAPP_H
