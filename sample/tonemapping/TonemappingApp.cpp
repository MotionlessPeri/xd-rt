//
// Created by Frank on 2024/2/22.
//

#include "TonemappingApp.h"
#include "backend/vulkan/MaterialFactoryVk.h"
#include "backend/vulkan/MaterialInstanceVk.h"
#include "backend/vulkan/MaterialTemplateVk.h"
#include "backend/vulkan/TextureFactoryVk.h"
#include "backend/vulkan/TextureVk.h"
#include "backend/vulkan/VulkanBuffer.h"
#include "backend/vulkan/VulkanCommandBuffer.h"
#include "backend/vulkan/VulkanDescriptorSet.h"
#include "backend/vulkan/VulkanDevice.h"
#include "backend/vulkan/VulkanFence.h"
#include "backend/vulkan/VulkanGlobal.h"
#include "backend/vulkan/VulkanImage.h"
#include "backend/vulkan/VulkanQueue.h"
#include "backend/vulkan/VulkanSwapchain.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#include "loader/TextureFactory.h"
namespace xd {
TonemappingApp::TonemappingApp(int width, int height, const char* title)
	: ImguiAppBase(width, height, title)
{
}

void TonemappingApp::initVulkan(const std::vector<const char*>& instanceEnabledExtensions)
{
	ImguiAppBase::initVulkan(instanceEnabledExtensions);
}
void TonemappingApp::loadAssets()
{
	const auto loadImage = [](const std::string& path) {
		const auto cpuImage = TextureFactory::get().loadUVTexture(path);
		auto texture = TextureFactoryVk::get().buildTexture(
			cpuImage, {.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT});
		VkImageMemoryBarrier barrier;
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.pNext = nullptr;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		texture->image->transitState(VK_PIPELINE_STAGE_TRANSFER_BIT,
									 VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, std::move(barrier));
		return texture;
	};
	inputImage = loadImage(R"(D:\uv_checker.jpg)");
}
void TonemappingApp::createResources()
{
	VkBufferCreateInfo bufferCi;
	bufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCi.pNext = nullptr;
	bufferCi.flags = 0;
	bufferCi.size = sizeof imageInfo;
	bufferCi.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfoBuffer = device->createBuffer(bufferCi, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
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
		computeSyncObjects[i] = {device->createSemaphore(semaphoreCi),
								 device->createSemaphore(semaphoreCi),
								 device->createFence(fenceCi)};
	}
}
void TonemappingApp::buildPipeline()
{
	mtlTemplate = MaterialFactoryVk::get().createTonemappingMaterial();
	mtlInstance = mtlTemplate->createInstance();
}
void TonemappingApp::buildFrameBuffers() {}

void TonemappingApp::recordCommandBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
										 uint32_t imageIndex)
{
	mtlTemplate->bindPipeline(cmdBuffer);
	mtlInstance->bindDescriptorSets(cmdBuffer);
	cmdBuffer->dispatch(16, 16, 1);
}

void TonemappingApp::updateResources(std::shared_ptr<VulkanCommandBuffer> cmdBuffer)
{
	imageInfo.width = width;
	imageInfo.height = height;
	imageInfo.enableTonemapping = enableTonemapping ? 1 : 0;
	imageInfoBuffer->setData(0, &imageInfo, sizeof imageInfo);
}
void TonemappingApp::bindResources(uint32_t swapchainImageIndex)
{
	const auto outputTexture = swapchainImages[swapchainImageIndex];
	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	outputTexture->image->transitState(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
									   VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, std::move(barrier));
	{
		const auto& set = mtlInstance->queryDescriptorSet("Uniform");
		set->bindResource(0, imageInfoBuffer->getBindingInfo());
	}
	{
		const auto& set = mtlInstance->queryDescriptorSet("Images");
		set->bindResource(0, inputImage->getBindingInfo(VK_IMAGE_LAYOUT_GENERAL));
		set->bindResource(1, outputTexture->getBindingInfo(VK_IMAGE_LAYOUT_GENERAL));
	}
	mtlInstance->updateDescriptorSets();
}
void TonemappingApp::render()
{
	computeSyncObjects[currentFrame].submitFence->wait();
	computeSyncObjects[currentFrame].submitFence->reset();

	const auto cmdBuffer = computeCmdBuffers[currentFrame];
	cmdBuffer->reset();
	cmdBuffer->beginCommandBuffer({VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
								   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr});
	updateResources(cmdBuffer);

	currentImageIndex =
		swapchain->acquireNextImage(computeSyncObjects[currentFrame].imageAcquireComplete, nullptr);
	bindResources(currentImageIndex);
	recordCommandBuffer(cmdBuffer, currentImageIndex);
	cmdBuffer->endCommandBuffer();

	SubmitInfoContainer submitData;
	submitData.waitingSemaphores.emplace_back(
		computeSyncObjects[currentFrame].imageAcquireComplete);
	submitData.waitingStages.emplace_back(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	submitData.signalingSemaphores.emplace_back(computeSyncObjects[currentFrame].computeComplete);
	submitData.signalingFence = computeSyncObjects[currentFrame].submitFence;
	cmdBuffer->submit(submitData);
}
void TonemappingApp::renderImgui()
{
	{
		ImGui::Begin("Hello World");
		ImGui::Checkbox("Enable Tonemapping", &enableTonemapping);
		ImGui::End();

		// const auto outputTexture = swapchainImages[currentImageIndex];
		// VkImageMemoryBarrier barrier;
		// barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		// barrier.pNext = nullptr;
		// barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		// barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		// barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
		// barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		// barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		// barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		// barrier.subresourceRange.baseMipLevel = 0;
		// barrier.subresourceRange.levelCount = 1;
		// barrier.subresourceRange.baseArrayLayer = 0;
		// barrier.subresourceRange.layerCount = 1;
		// outputTexture->image->transitState(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		//								   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		//								   std::move(barrier));
	}
	imguiSyncObjects[currentFrame].submitFence->wait();
	imguiSyncObjects[currentFrame].submitFence->reset();
	auto cmdBuffer = graphicCmdBuffers[currentFrame];
	cmdBuffer->reset();
	cmdBuffer->beginCommandBuffer({VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
								   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr});
	recordImguiCmdBuffer(cmdBuffer);

	// transit to present
	// VkImageMemoryBarrier barrier;
	// barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	// barrier.pNext = nullptr;
	// barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	// barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	// barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	// barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	// barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	// barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	// barrier.subresourceRange.baseMipLevel = 0;
	// barrier.subresourceRange.levelCount = 1;
	// barrier.subresourceRange.baseArrayLayer = 0;
	// barrier.subresourceRange.layerCount = 1;
	// swapchainImages[currentImageIndex]->image->transitState(
	//	cmdBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
	//	std::move(barrier));
	cmdBuffer->endCommandBuffer();
	SubmitInfoContainer submitData;
	submitData.waitingSemaphores.emplace_back(computeSyncObjects[currentFrame].computeComplete);
	submitData.waitingStages.emplace_back(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	submitData.signalingSemaphores.emplace_back(imguiSyncObjects[currentFrame].renderComplete);
	submitData.signalingFence = imguiSyncObjects[currentFrame].submitFence;
	cmdBuffer->submit(submitData);
}

void TonemappingApp::present()
{
	ImguiAppBase::present();
}
}  // namespace xd

int main(int argc, char* argv[])
{
	constexpr int WIDTH = 512;
	constexpr int HEIGHT = 512;
	const char* TITLE = "tonemapping mtl test";
	xd::TonemappingApp app{WIDTH, HEIGHT, TITLE};
	app.init();
	app.run();
	return 0;
}
