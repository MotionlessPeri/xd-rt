//
// Created by Frank on 2024/2/23.
//

#include "BasicMultipassApp.h"

#include <glm/gtc/matrix_transform.hpp>

#include "backend/vulkan/LightManager.h"
#include "backend/vulkan/MaterialFactoryVk.h"
#include "backend/vulkan/MaterialInstanceVk.h"
#include "backend/vulkan/MaterialTemplateVk.h"
#include "backend/vulkan/ModelFactoryVk.h"
#include "backend/vulkan/TextureFactoryVk.h"
#include "backend/vulkan/TextureVk.h"
#include "backend/vulkan/TriangleMeshVk.h"
#include "backend/vulkan/VulkanBuffer.h"
#include "backend/vulkan/VulkanCommandBuffer.h"
#include "backend/vulkan/VulkanDescriptorSet.h"
#include "backend/vulkan/VulkanDevice.h"
#include "backend/vulkan/VulkanFence.h"
#include "backend/vulkan/VulkanFrameBuffer.h"
#include "backend/vulkan/VulkanImage.h"
#include "backend/vulkan/VulkanQueue.h"
#include "backend/vulkan/VulkanRenderPass.h"
#include "backend/vulkan/VulkanSwapchain.h"
#include "imgui.h"
#include "loader/TextureFactory.h"
#include "model/Sphere.h"
namespace xd {
BasicMultipassApp::BasicMultipassApp(int width, int height, const char* title)
	: ImguiAppBase(width, height, title)
{
}
void BasicMultipassApp::loadAssets()
{
	auto mesh = std::make_shared<Sphere>(1.f)->getTriangulatedMesh();
	model = ModelFactoryVk::get().buildTriangleMesh(mesh);

	const auto loadImage = [](const std::string& path) {
		const auto cpuImage = TextureFactory::get().loadUVTexture(path);
		auto texture = TextureFactoryVk::get().buildTexture(cpuImage);
		VkImageMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		texture->image->transitState(VK_PIPELINE_STAGE_TRANSFER_BIT,
									 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, std::move(barrier));
		return texture;
	};
	mtlResources.diffuse = loadImage(R"(D:\uv_checker.jpg)");
	mtlResources.normal = loadImage(R"(D:\normal_map.png)");
}
void BasicMultipassApp::createResources()
{
	// create render pass
	{
		RenderPassDesc renderPassDesc;
		VkAttachmentDescription2 colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
		colorAttachment.pNext = nullptr;
		colorAttachment.format = swapchain->getFormat().format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		renderPassDesc.attachments.emplace_back(colorAttachment);
		VkAttachmentDescription2 depthAttach{};
		depthAttach.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
		depthAttach.pNext = nullptr;
		depthAttach.format = VK_FORMAT_D32_SFLOAT;
		depthAttach.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttach.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttach.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttach.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttach.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		renderPassDesc.attachments.emplace_back(depthAttach);

		SubpassDesc subpassDesc;
		VkAttachmentReference2 colorAttachmentRef{};
		colorAttachmentRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
		colorAttachmentRef.pNext = nullptr;
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachmentRef.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subpassDesc.colorRefs.emplace_back(colorAttachmentRef);

		VkAttachmentReference2 depthAttachRef{};
		depthAttachRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
		depthAttachRef.pNext = nullptr;
		depthAttachRef.attachment = 1;
		depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachRef.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		subpassDesc.depthStencilRefs.emplace_back(depthAttachRef);

		subpassDesc.desc.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
		subpassDesc.desc.pNext = nullptr;
		subpassDesc.desc.flags = 0;
		subpassDesc.desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDesc.desc.viewMask = 0;
		subpassDesc.desc.inputAttachmentCount = subpassDesc.inputRefs.size();
		subpassDesc.desc.pInputAttachments =
			subpassDesc.inputRefs.empty() ? nullptr : subpassDesc.inputRefs.data();
		subpassDesc.desc.colorAttachmentCount = subpassDesc.colorRefs.size();
		subpassDesc.desc.pColorAttachments =
			subpassDesc.colorRefs.empty() ? nullptr : subpassDesc.colorRefs.data();
		subpassDesc.desc.pResolveAttachments = nullptr;
		subpassDesc.desc.pDepthStencilAttachment =
			subpassDesc.depthStencilRefs.empty() ? nullptr : subpassDesc.depthStencilRefs.data();
		subpassDesc.desc.preserveAttachmentCount = subpassDesc.preserveAttaches.size();
		subpassDesc.desc.pPreserveAttachments =
			subpassDesc.preserveAttaches.empty() ? nullptr : subpassDesc.preserveAttaches.data();
		renderPassDesc.subpasses.emplace_back(subpassDesc);

		VkSubpassDependency2 colorAttachExtDep{};
		colorAttachExtDep.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
		colorAttachExtDep.pNext = nullptr;
		colorAttachExtDep.srcSubpass = VK_SUBPASS_EXTERNAL;
		colorAttachExtDep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		colorAttachExtDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		colorAttachExtDep.srcAccessMask = 0;
		colorAttachExtDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		colorAttachExtDep.dependencyFlags = 0;
		colorAttachExtDep.viewOffset = 0;
		renderPassDesc.dependencys.emplace_back(colorAttachExtDep);

		VkSubpassDependency2 depthAttachExtDep{};
		depthAttachExtDep.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
		depthAttachExtDep.pNext = nullptr;
		depthAttachExtDep.srcSubpass = VK_SUBPASS_EXTERNAL;
		depthAttachExtDep.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		depthAttachExtDep.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		depthAttachExtDep.srcAccessMask = 0;
		depthAttachExtDep.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		depthAttachExtDep.dependencyFlags = 0;
		depthAttachExtDep.viewOffset = 0;
		renderPassDesc.dependencys.emplace_back(depthAttachExtDep);

		std::vector<VkSubpassDescription2> vkSubpasses{subpassDesc.desc};
		renderPassDesc.ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
		renderPassDesc.ci.pNext = nullptr;
		renderPassDesc.ci.flags = 0;
		renderPassDesc.ci.attachmentCount = renderPassDesc.attachments.size();
		renderPassDesc.ci.pAttachments =
			renderPassDesc.attachments.empty() ? nullptr : renderPassDesc.attachments.data();
		renderPassDesc.ci.subpassCount = vkSubpasses.size();
		renderPassDesc.ci.pSubpasses = vkSubpasses.empty() ? nullptr : vkSubpasses.data();
		renderPassDesc.ci.dependencyCount = renderPassDesc.dependencys.size();
		renderPassDesc.ci.pDependencies =
			renderPassDesc.dependencys.empty() ? nullptr : renderPassDesc.dependencys.data();
		renderPassDesc.ci.correlatedViewMaskCount = 0;
		renderPassDesc.ci.pCorrelatedViewMasks = nullptr;
		graphicRenderPass = device->createRenderPass(renderPassDesc);
	}

	// create material resources
	{
		auto& mtlFactory = MaterialFactoryVk::get();
		lambertianMtl.mtlTemplate =
			mtlFactory.createLambertianMaterial(graphicRenderPass->getSubpasses().front());
		lambertianMtl.mtlInstance = lambertianMtl.mtlTemplate->createInstance();

		tonemappingMtl.mtlTemplate = mtlFactory.createTonemappingMaterial();
		tonemappingMtl.mtlInstance = tonemappingMtl.mtlTemplate->createInstance();

		uniformData.model = glm::mat4{1.f};
		const auto cameraPos = glm::vec3{1.5, 0, 0};
		uniformData.view = glm::lookAt(cameraPos, glm::vec3{0, 0, 0}, -glm::vec3{0, 0, 1});
		uniformData.proj = glm::perspective(90.f, (float)width / height, 0.1f, 100.f);
		uniformData.normalTransform = glm::transpose(glm::inverse(uniformData.model));
		uniformData.cameraWorldPos = glm::vec4{cameraPos, 1.f};

		{
			VkBufferCreateInfo bufferCi;
			bufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCi.pNext = nullptr;
			bufferCi.flags = 0;
			bufferCi.size = sizeof(uniformData);
			bufferCi.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			bufferCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			uniformBuffer = device->createBuffer(bufferCi, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		}

		{
			imageInfo.width = width;
			imageInfo.height = height;
			VkBufferCreateInfo bufferCi;
			bufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCi.pNext = nullptr;
			bufferCi.flags = 0;
			bufferCi.size = sizeof imageInfo;
			bufferCi.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			bufferCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfoBuffer = device->createBuffer(bufferCi, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		}

		lightManager = std::make_shared<LightManager>(device);
		lightManager->addPointLight({cameraPos, {1, 1, 1}});
	}

	// create sync objects
	graphicSyncObjects.resize(frameCount);
	computeSyncObjects.resize(frameCount);
	for (const auto i : std::views::iota(0u, frameCount)) {
		VkSemaphoreCreateInfo semaphoreCi;
		semaphoreCi.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCi.pNext = nullptr;
		semaphoreCi.flags = 0;
		VkFenceCreateInfo fenceCi;
		fenceCi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCi.pNext = nullptr;
		fenceCi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		graphicSyncObjects[i] = {device->createSemaphore(semaphoreCi),
								 device->createSemaphore(semaphoreCi),
								 device->createFence(fenceCi)};
		computeSyncObjects[i] = {device->createSemaphore(semaphoreCi),
								 device->createSemaphore(semaphoreCi),
								 device->createFence(fenceCi)};
	}

	// create swapchain images
	const auto& rawOutputImages = swapchain->getSwapchainImages();
	swapchainImages.resize(frameCount);
	std::ranges::transform(
		rawOutputImages, swapchainImages.begin(), [](const auto& swapchainImage) {
			auto ret = std::make_shared<TextureVk>();
			VkImageMemoryBarrier barrier;
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.srcAccessMask = VK_ACCESS_NONE;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			swapchainImage.image->transitState(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
											   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
											   std::move(barrier));
			ret->image = swapchainImage.image;
			ret->imageView = swapchainImage.view;
			ret->sampler = nullptr;
			return ret;
		});
}

void BasicMultipassApp::buildPipeline() {}

void BasicMultipassApp::buildFrameBuffers()
{
	graphicFrameBuffers.resize(frameCount);
	frameBufferResources.resize(frameCount);
	const auto& swapchainImages = swapchain->getSwapchainImages();
	const auto swapchainExtent = swapchain->getExtent();
	for (const auto i : std::views::iota(0ull, swapchainImages.size())) {
		const auto& image = swapchainImages[i];
		VkImageCreateInfo imageCi;
		imageCi.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCi.pNext = nullptr;
		imageCi.flags = 0;
		imageCi.imageType = VK_IMAGE_TYPE_2D;
		imageCi.format = VK_FORMAT_D32_SFLOAT;
		imageCi.extent.width = swapchainExtent.width;
		imageCi.extent.height = swapchainExtent.height;
		imageCi.extent.depth = 1;
		imageCi.mipLevels = 1;
		imageCi.arrayLayers = 1;
		imageCi.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCi.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCi.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCi.queueFamilyIndexCount = 0;
		imageCi.pQueueFamilyIndices = nullptr;
		imageCi.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		auto depthStencilImage = device->createImage(imageCi, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VkImageViewCreateInfo viewCi;
		viewCi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCi.pNext = nullptr;
		viewCi.flags = 0;
		viewCi.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCi.format = VK_FORMAT_D32_SFLOAT;
		viewCi.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCi.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCi.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCi.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewCi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewCi.subresourceRange.baseMipLevel = 0;
		viewCi.subresourceRange.levelCount = 1;
		viewCi.subresourceRange.baseArrayLayer = 0;
		viewCi.subresourceRange.layerCount = 1;
		auto depthStencilImageView = depthStencilImage->createImageView(std::move(viewCi));
		std::vector<std::shared_ptr<VulkanImageView>> imageViews{image.view, depthStencilImageView};
		frameBufferResources[i].colorAttach = image.image;
		frameBufferResources[i].colorAttachView = image.view;
		frameBufferResources[i].depthStencilAttach = depthStencilImage;
		frameBufferResources[i].depthStencilView = depthStencilImageView;
		const auto imageViewHandleView =
			imageViews | std::views::transform([](const auto& view) { return view->imageView; });
		const std::vector<VkImageView> attachmentHandles{imageViewHandleView.begin(),
														 imageViewHandleView.end()};
		VkFramebufferCreateInfo ci;
		ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = 0;
		ci.renderPass = graphicRenderPass->pass;
		ci.attachmentCount = attachmentHandles.size();
		ci.pAttachments = attachmentHandles.data();
		ci.width = width;
		ci.height = height;
		ci.layers = 1;
		graphicFrameBuffers[i] = graphicRenderPass->createFrameBuffer(std::move(ci));
	}
}

void BasicMultipassApp::recordCommandBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
											uint32_t imageIndex)
{
}

void BasicMultipassApp::updateResources(std::shared_ptr<VulkanCommandBuffer> cmdBuffer)
{
	imageInfo.width = width;
	imageInfo.height = height;
	imageInfo.enableTonemapping = enableTonemapping ? 1 : 0;
	imageInfoBuffer->setData(0, &imageInfo, sizeof imageInfo);

	uniformBuffer->setData(0, &uniformData, sizeof uniformData);
}
void BasicMultipassApp::render()
{
	updateResources(nullptr);
	// render graphic pass
	{
		graphicSyncObjects[currentFrame].submitFence->wait();
		graphicSyncObjects[currentFrame].submitFence->reset();
		currentImageIndex = swapchain->acquireNextImage(
			graphicSyncObjects[currentFrame].imageAcquireComplete, nullptr);
		auto graphicCmdBuffer = graphicCmdBuffers[currentFrame];
		bindGraphicPipelineResources();
		graphicCmdBuffer->reset();
		graphicCmdBuffer->beginCommandBuffer({VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
											  VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
											  nullptr});
		recordGraphicCmdBuffer(graphicCmdBuffer);
		graphicCmdBuffer->endCommandBuffer();

		SubmitInfoContainer submitData;
		submitData.waitingSemaphores.emplace_back(
			graphicSyncObjects[currentFrame].imageAcquireComplete);
		submitData.waitingStages.emplace_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		submitData.signalingSemaphores.emplace_back(
			graphicSyncObjects[currentFrame].renderComplete);
		submitData.signalingFence = graphicSyncObjects[currentFrame].submitFence;
		graphicCmdBuffer->submit(submitData);
	}
	// render compute pass
	{
		computeSyncObjects[currentFrame].submitFence->wait();
		computeSyncObjects[currentFrame].submitFence->reset();
		bindComputePipelineResources();
		auto computeCmdBuffer = computeCmdBuffers[currentFrame];
		computeCmdBuffer->reset();
		computeCmdBuffer->beginCommandBuffer({VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
											  VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
											  nullptr});
		recordComputeCmdBuffer(computeCmdBuffer);
		computeCmdBuffer->endCommandBuffer();

		SubmitInfoContainer submitData;
		submitData.waitingSemaphores.emplace_back(graphicSyncObjects[currentFrame].renderComplete);
		submitData.waitingStages.emplace_back(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		submitData.signalingSemaphores.emplace_back(
			computeSyncObjects[currentFrame].renderComplete);
		submitData.signalingFence = computeSyncObjects[currentFrame].submitFence;
		computeCmdBuffer->submit(submitData);
	}
}

void BasicMultipassApp::renderImgui()
{
	ImGui::Begin("Hello World");
	ImGui::Checkbox("Enable Tonemapping", &enableTonemapping);
	ImGui::End();

	// render imgui pass
	{
		imguiSyncObjects[currentFrame].submitFence->wait();
		imguiSyncObjects[currentFrame].submitFence->reset();
		auto imguiCmdBuffer = imguiCmdBuffers[currentFrame];
		imguiCmdBuffer->reset();
		imguiCmdBuffer->beginCommandBuffer({VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
											VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr});
		recordImguiCmdBuffer(imguiCmdBuffer);
		imguiCmdBuffer->endCommandBuffer();

		SubmitInfoContainer submitData;
		submitData.waitingSemaphores.emplace_back(computeSyncObjects[currentFrame].renderComplete);
		submitData.waitingStages.emplace_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		submitData.signalingSemaphores.emplace_back(imguiSyncObjects[currentFrame].renderComplete);
		submitData.signalingFence = imguiSyncObjects[currentFrame].submitFence;
		imguiCmdBuffer->submit(submitData);
	}
}

void BasicMultipassApp::bindGraphicPipelineResources()
{
	{
		const auto& set = lambertianMtl.mtlInstance->queryDescriptorSet("Scene");
		set->bindResource(0, uniformBuffer->getBindingInfo());
	}
	{
		const auto& set = lambertianMtl.mtlInstance->queryDescriptorSet("Material");
		set->bindResource(
			0, mtlResources.diffuse->getBindingInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
		set->bindResource(
			1, mtlResources.normal->getBindingInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
	}
	{
		lightManager->bindLightInfos(lambertianMtl.mtlInstance);
	}
	lambertianMtl.mtlInstance->updateDescriptorSets();
}
void BasicMultipassApp::bindComputePipelineResources()
{
	const auto outputTexture = swapchainImages[currentImageIndex];
	{
		const auto& set = tonemappingMtl.mtlInstance->queryDescriptorSet("Uniform");
		set->bindResource(0, imageInfoBuffer->getBindingInfo());
	}
	{
		const auto& set = tonemappingMtl.mtlInstance->queryDescriptorSet("Images");
		set->bindResource(0, outputTexture->getBindingInfo(VK_IMAGE_LAYOUT_GENERAL));
		set->bindResource(1, outputTexture->getBindingInfo(VK_IMAGE_LAYOUT_GENERAL));
	}
	tonemappingMtl.mtlInstance->updateDescriptorSets();
}
void BasicMultipassApp::bindResources()
{
	bindGraphicPipelineResources();
	bindComputePipelineResources();
}

void BasicMultipassApp::recordGraphicCmdBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer)
{
	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	swapchainImages[currentImageIndex]->image->transitState(
		cmdBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		std::move(barrier));

	const auto swapchainExtent = swapchain->getExtent();
	{
		std::array<VkClearValue, 2> clearValues;
		clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
		clearValues[1].depthStencil = {1.f, 0};
		VkRenderPassBeginInfo info;
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.pNext = nullptr;
		info.renderPass = graphicRenderPass->pass;
		info.framebuffer = graphicFrameBuffers[currentImageIndex]->frameBuffer;
		info.renderArea.offset = {0, 0};
		info.renderArea.extent = swapchainExtent;
		info.clearValueCount = clearValues.size();
		info.pClearValues = clearValues.data();
		cmdBuffer->beginRenderPass(info, VK_SUBPASS_CONTENTS_INLINE);
	}
	lambertianMtl.mtlTemplate->bindPipeline(cmdBuffer);
	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapchainExtent.width);
	viewport.height = static_cast<float>(swapchainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	cmdBuffer->setViewport(viewport);

	VkRect2D scissor;
	scissor.offset = {0, 0};
	scissor.extent = swapchainExtent;
	cmdBuffer->setScissor(scissor);

	lambertianMtl.mtlInstance->bindDescriptorSets(cmdBuffer);

	model->bind(cmdBuffer);
	model->draw(cmdBuffer);

	cmdBuffer->endRenderPass();
}
void BasicMultipassApp::recordComputeCmdBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer)
{
	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	swapchainImages[currentImageIndex]->image->transitState(
		cmdBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		std::move(barrier));

	tonemappingMtl.mtlTemplate->bindPipeline(cmdBuffer);
	tonemappingMtl.mtlInstance->bindDescriptorSets(cmdBuffer);
	cmdBuffer->dispatch(16, 16, 1);
}

void BasicMultipassApp::present()
{
	QueuePresentInfoContainer desc;
	desc.waitSemaphores.emplace_back(imguiSyncObjects[currentFrame].renderComplete);
	desc.swapchains.emplace_back(swapchain);
	desc.imageIndices.emplace_back(currentImageIndex);
	presentQueue->present(desc);
	currentFrame = (currentFrame + 1) % frameCount;
}
}  // namespace xd

int main()
{
	constexpr int WIDTH = 1000;
	constexpr int HEIGHT = 800;
	const char* TITLE = "glfw basic multipass test";
	xd::BasicMultipassApp app{WIDTH, HEIGHT, TITLE};
	app.init();
	app.run();
	return 0;
}