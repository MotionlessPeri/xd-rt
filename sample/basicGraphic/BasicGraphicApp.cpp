//
// Created by Frank on 2024/1/10.
//

#include "BasicGraphicApp.h"
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ranges>
#include <sstream>
#include <string>
#include "backend/vulkan/FrameGraph.h"
#include "backend/vulkan/MaterialFactoryVk.h"
#include "backend/vulkan/MaterialInstanceVk.h"
#include "backend/vulkan/MaterialTemplateVk.h"
#include "backend/vulkan/ModelFactoryVk.h"
#include "backend/vulkan/TextureFactoryVk.h"
#include "backend/vulkan/TextureVk.h"
#include "backend/vulkan/TriangleMeshVk.h"
#include "backend/vulkan/VulkanBuffer.h"
#include "backend/vulkan/VulkanCommandBuffer.h"
#include "backend/vulkan/VulkanCommandPool.h"
#include "backend/vulkan/VulkanDescriptorPool.h"
#include "backend/vulkan/VulkanDescriptorSet.h"
#include "backend/vulkan/VulkanDescriptorSetLayout.h"
#include "backend/vulkan/VulkanDevice.h"
#include "backend/vulkan/VulkanFence.h"
#include "backend/vulkan/VulkanFrameBuffer.h"
#include "backend/vulkan/VulkanGlobal.h"
#include "backend/vulkan/VulkanGraphicsPipeline.h"
#include "backend/vulkan/VulkanImage.h"
#include "backend/vulkan/VulkanImageView.h"
#include "backend/vulkan/VulkanQueue.h"
#include "backend/vulkan/VulkanRenderPass.h"
#include "backend/vulkan/VulkanShader.h"
#include "backend/vulkan/VulkanSwapchain.h"
#include "loader/ObjMeshLoader.h"
#include "loader/TextureFactory.h"
#include "model/Sphere.h"

namespace xd {
BasicGraphicApp::BasicGraphicApp(int width, int height, const char* title)
	: VulkanGLFWAppBase(width, height, title)
{
}

void BasicGraphicApp::initVulkan(const std::vector<const char*>& instanceEnabledExtensions)
{
	VulkanGlobal::init(std::move(instanceEnabledExtensions), {"VK_LAYER_KHRONOS_validation"}, true,
					   glfwGetWin32Window(window), width, height,
					   VkPhysicalDeviceFeatures{.geometryShader = true, .samplerAnisotropy = true},
					   {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
					   {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
					   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	instance = VulkanGlobal::instance;
	physicalDevice = VulkanGlobal::physicalDevice;
	device = VulkanGlobal::device;
	swapchain = VulkanGlobal::swapchain;
	presentQueue = VulkanGlobal::presentQueue;
	graphicQueue = VulkanGlobal::graphicQueue;
	computeQueue = VulkanGlobal::computeQueue;
	frameCount = swapchain->getSwapchainImageCount();
	// allocate command buffers
	VkCommandPoolCreateInfo poolCi;
	poolCi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCi.pNext = nullptr;
	poolCi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolCi.queueFamilyIndex = VulkanGlobal::graphicQueue->getQueueFamilyIndex();
	graphicCmdPool = device->createCommandPool(std::move(poolCi));
	VkCommandBufferAllocateInfo cmdBufferInfo;
	cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferInfo.pNext = nullptr;
	cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferInfo.commandBufferCount = frameCount;
	graphicCmdBuffers = graphicCmdPool->allocateCommandBuffers(std::move(cmdBufferInfo));
	// create sync objects
	for ([[maybe_unused]] const auto i : std::views::iota(0u, frameCount)) {
		VkSemaphoreCreateInfo semaphoreCi;
		semaphoreCi.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCi.pNext = nullptr;
		semaphoreCi.flags = 0;
		VkFenceCreateInfo fenceCi;
		fenceCi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCi.pNext = nullptr;
		fenceCi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		syncObjects.emplace_back(device->createSemaphore(semaphoreCi),
								 device->createSemaphore(semaphoreCi),
								 device->createFence(fenceCi));
	}
}

void BasicGraphicApp::handleInput(GLFWwindow* window) {}

void BasicGraphicApp::loadAssets()
{
	// ObjMeshLoader loader;
	// auto mesh = loader.load(R"(D:\qem-test.obj)");

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

void BasicGraphicApp::createResources()
{
	uniformData.model = glm::mat4{1.f};
	const auto cameraPos = glm::vec3{1.5, 0, 0};
	uniformData.view = glm::lookAt(cameraPos, glm::vec3{0, 0, 0}, -glm::vec3{0, 0, 1});
	uniformData.proj = glm::perspective(90.f, (float)width / height, 0.1f, 100.f);
	uniformData.normalTransform = glm::transpose(glm::inverse(uniformData.model));
	uniformData.cameraWorldPos = glm::vec4{cameraPos, 1.f};

	VkBufferCreateInfo bufferCi;
	bufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCi.pNext = nullptr;
	bufferCi.flags = 0;
	bufferCi.size = sizeof(uniformData);
	bufferCi.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	uniformBuffer = device->createBuffer(bufferCi, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	lightManager = std::make_shared<LightManager>(device);
	lightManager->addPointLight({cameraPos, {1, 1, 1}});

	buildMaterial();
}

void BasicGraphicApp::buildPipeline()
{
	FrameGraphBuilder builder;
	auto& subpass = builder.addPass("main pass");
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
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentReference2 colorAttachmentRef{};
	colorAttachmentRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
	colorAttachmentRef.pNext = nullptr;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachmentRef.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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
	fgHandles.color = subpass.addColorAttach(
		std::move(colorAttachmentRef), std::move(colorAttachment), std::move(colorAttachExtDep));
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
	VkAttachmentReference2 depthAttachRef{};
	depthAttachRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
	depthAttachRef.pNext = nullptr;
	depthAttachRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthAttachRef.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
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
	fgHandles.depth = subpass.addDepthAttach(std::move(depthAttachRef), std::move(depthAttach),
											 std::move(depthAttachExtDep));
	frameGraph = builder.build(device);
}

void BasicGraphicApp::buildMaterial()
{
	mtlTemplate = MaterialFactoryVk::get().createLambertian(frameGraph->subpasses.front().subpass);
	mtlInstance = mtlTemplate->createInstance();
}

void BasicGraphicApp::bindResources()
{
	{
		const auto& set = mtlInstance->queryDescriptorSet("Scene");
		set->bindResource(0, uniformBuffer->getBindingInfo());
	}
	{
		const auto& set = mtlInstance->queryDescriptorSet("Material");
		set->bindResource(
			0, mtlResources.diffuse->getBindingInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
		set->bindResource(
			1, mtlResources.normal->getBindingInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
	}
	{
		lightManager->bindLightInfos(mtlInstance);
	}
	mtlInstance->updateDescriptorSets();
}

void BasicGraphicApp::buildFrameBuffers()
{
	frameBuffers.resize(frameCount);
	frameBufferResources.resize(frameCount);
	const auto& swapchainImages = swapchain->getSwapchainImages();
	const auto swapchainExtent = swapchain->getExtent();
	for (const auto i : std::views::iota(0ull, swapchainImages.size())) {
		const auto& image = swapchainImages[i];
		frameBufferResources[i].colorAttach = image.image;
		frameBufferResources[i].colorAttachView = image.view;
		frameGraph->bindAttachmentBuffer(fgHandles.color, image.view);
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
		frameBufferResources[i].depthStencilAttach =
			device->createImage(imageCi, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
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
		frameBufferResources[i].depthStencilView =
			frameBufferResources[i].depthStencilAttach->createImageView(std::move(viewCi));
		frameGraph->bindAttachmentBuffer(fgHandles.depth, frameBufferResources[i].depthStencilView);
		frameGraph->buildFrameBuffer(swapchainExtent.width, swapchainExtent.height);
		frameBuffers[i] = frameGraph->frameBuffer;
	}
}

void BasicGraphicApp::recordCommandBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
										  uint32_t imageIndex)
{
	const auto swapchainExtent = swapchain->getExtent();
	{
		std::array<VkClearValue, 2> clearValues;
		clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
		clearValues[1].depthStencil = {1.f, 0};
		VkRenderPassBeginInfo info;
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.pNext = nullptr;
		info.renderPass = frameGraph->renderPass->pass;
		info.framebuffer = frameBuffers[imageIndex]->frameBuffer;
		info.renderArea.offset = {0, 0};
		info.renderArea.extent = swapchainExtent;
		info.clearValueCount = clearValues.size();
		info.pClearValues = clearValues.data();
		cmdBuffer->beginRenderPass(info, VK_SUBPASS_CONTENTS_INLINE);
	}
	mtlTemplate->bindPipeline(cmdBuffer);
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

	mtlInstance->bindDescriptorSets(cmdBuffer);

	model->bind(cmdBuffer);
	model->draw(cmdBuffer);

	cmdBuffer->endRenderPass();
}

void BasicGraphicApp::updateResources(std::shared_ptr<VulkanCommandBuffer> cmdBuffer)
{
	// VkBufferMemoryBarrier barrier;
	// barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	// barrier.pNext = nullptr;
	// barrier.srcAccessMask = VK_ACCESS_UNIFORM_READ_BIT;
	// barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	// barrier.srcQueueFamilyIndex = 0;
	// barrier.dstQueueFamilyIndex = 0;
	// barrier.offset = 0;
	// barrier.size = VK_WHOLE_SIZE;
	// uniformBuffer->transitState(
	//	cmdBuffer, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
	//	VK_PIPELINE_STAGE_TRANSFER_BIT, std::move(barrier));
	uniformBuffer->setData(0, &uniformData, sizeof(uniformData));
}

void BasicGraphicApp::draw()
{
	// waiting for fence before using it
	syncObjects[currentFrame].submitFence->wait();
	syncObjects[currentFrame].submitFence->reset();
	const auto cmdBuffer = graphicCmdBuffers[currentFrame];
	cmdBuffer->reset();
	cmdBuffer->beginCommandBuffer({VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
								   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr});
	updateResources(cmdBuffer);
	bindResources();

	const auto imageIndex =
		swapchain->acquireNextImage(syncObjects[currentFrame].imageAcquireComplete, nullptr);
	recordCommandBuffer(cmdBuffer, imageIndex);
	cmdBuffer->endCommandBuffer();

	SubmitInfoContainer submitData;
	submitData.waitingSemaphores.emplace_back(syncObjects[currentFrame].imageAcquireComplete);
	submitData.waitingStages.emplace_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	submitData.signalingSemaphores.emplace_back(syncObjects[currentFrame].renderComplete);
	submitData.signalingFence = syncObjects[currentFrame].submitFence;
	cmdBuffer->submit(submitData);

	QueuePresentInfoContainer desc;
	desc.waitSemaphores.emplace_back(syncObjects[currentFrame].renderComplete);
	desc.swapchains.emplace_back(swapchain);
	desc.imageIndices.emplace_back(imageIndex);
	presentQueue->present(desc);
	currentFrame = (currentFrame + 1) % frameCount;
}
}  // namespace xd

int main(int argc, char* argv[])
{
	constexpr int WIDTH = 1000;
	constexpr int HEIGHT = 800;
	const char* TITLE = "glfw basic graphic test";
	xd::BasicGraphicApp app{WIDTH, HEIGHT, TITLE};
	app.init();
	app.run();
}
