//
// Created by Frank on 2024/1/10.
//

#include "BasicComputeApp.h"
#include <fstream>
#include <string>

#include "backend/vulkan/MaterialInstanceVk.h"
#include "backend/vulkan/MaterialTemplateVk.h"
#include "backend/vulkan/TextureFactoryVk.h"
#include "backend/vulkan/TextureVk.h"
#include "backend/vulkan/VulkanBuffer.h"
#include "backend/vulkan/VulkanCommandBuffer.h"
#include "backend/vulkan/VulkanCommandPool.h"
#include "backend/vulkan/VulkanComputePipeline.h"
#include "backend/vulkan/VulkanDescriptorSet.h"
#include "backend/vulkan/VulkanDevice.h"
#include "backend/vulkan/VulkanFence.h"
#include "backend/vulkan/VulkanGlobal.h"
#include "backend/vulkan/VulkanImage.h"
#include "backend/vulkan/VulkanQueue.h"
#include "backend/vulkan/VulkanShader.h"
#include "backend/vulkan/VulkanSwapchain.h"
#include "loader/TextureFactory.h"
namespace xd {
BasicComputeApp::BasicComputeApp(int width, int height, const char* title)
	: VulkanGLFWAppBase(width, height, title)
{
}

void BasicComputeApp::initVulkan(const std::vector<const char*>& instanceEnabledExtensions)
{
	VulkanGlobal::init(std::move(instanceEnabledExtensions), {"VK_LAYER_KHRONOS_validation"}, true,
					   glfwGetWin32Window(window), width, height,
					   VkPhysicalDeviceFeatures{.geometryShader = true, .samplerAnisotropy = true},
					   {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
					   {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
					   VK_IMAGE_USAGE_STORAGE_BIT);
	instance = VulkanGlobal::instance;
	physicalDevice = VulkanGlobal::physicalDevice;
	device = VulkanGlobal::device;
	swapchain = VulkanGlobal::swapchain;
	presentQueue = VulkanGlobal::presentQueue;
	graphicQueue = VulkanGlobal::graphicQueue;
	computeQueue = VulkanGlobal::computeQueue;
	frameCount = swapchain->getSwapchainImageCount();
}

void BasicComputeApp::loadAssets()
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
void BasicComputeApp::createResources()
{
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
	// create cmd pool and buffers
	VkCommandPoolCreateInfo poolCi;
	poolCi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCi.pNext = nullptr;
	poolCi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolCi.queueFamilyIndex = computeQueue->getQueueFamilyIndex();
	computeCmdPool = device->createCommandPool(std::move(poolCi));
	VkCommandBufferAllocateInfo cmdBufferInfo;
	cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferInfo.pNext = nullptr;
	cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferInfo.commandBufferCount = frameCount;
	computeCmdBuffers = computeCmdPool->allocateCommandBuffers(std::move(cmdBufferInfo));
	// create uniform buffer
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
void BasicComputeApp::buildPipeline()
{
	const auto createShader = [&](const std::string& path,
								  VkShaderStageFlagBits stage) -> std::shared_ptr<VulkanShader> {
		std::ifstream fstream{path, std::ios::ate | std::ios::binary};
		const std::size_t fileSize = fstream.tellg();
		fstream.seekg(0);
		std::vector<char> code(fileSize);
		fstream.read(code.data(), fileSize);
		VkShaderModuleCreateInfo ci;
		ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = 0;
		ci.codeSize = code.size();
		ci.pCode = reinterpret_cast<const uint32_t*>(code.data());
		return device->createShader(ci, stage);
	};
	using namespace std::string_literals;
	const auto computeShader =
		createShader(PROJECT_ROOT + "/shader/test.comp.spv"s, VK_SHADER_STAGE_COMPUTE_BIT);

	std::vector<std::shared_ptr<VulkanDescriptorSetLayout>> layouts;
	std::unordered_map<std::string, uint32_t> layoutNameToIndex;
	{
		// declare imageInfo(set 0)
		DescriptorSetLayoutDesc layoutDesc;
		layoutDesc.bindings = std::vector<VkDescriptorSetLayoutBinding>{
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
		layoutDesc.ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutDesc.ci.pNext = nullptr;
		layoutDesc.ci.flags = 0;
		layoutDesc.ci.bindingCount = layoutDesc.bindings.size();
		layoutDesc.ci.pBindings = layoutDesc.bindings.data();
		std::unordered_map<std::string, uint32_t> nameToBinding{{"imageInfo", 0}};
		layoutNameToIndex.insert({"Uniform", 0});
		layouts.emplace_back(
			device->createDescriptorSetLayout(layoutDesc, std::move(nameToBinding)));
	}
	{
		// declare input and output(set 1)
		DescriptorSetLayoutDesc layoutDesc;
		layoutDesc.bindings = std::vector<VkDescriptorSetLayoutBinding>{
			{0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr},
			{1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}};
		layoutDesc.ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutDesc.ci.pNext = nullptr;
		layoutDesc.ci.flags = 0;
		layoutDesc.ci.bindingCount = layoutDesc.bindings.size();
		layoutDesc.ci.pBindings = layoutDesc.bindings.data();
		std::unordered_map<std::string, uint32_t> nameToBinding{{"InputImage", 0},
																{"OutputImage", 1}};
		layoutNameToIndex.insert({"Images", 1});
		layouts.emplace_back(
			device->createDescriptorSetLayout(layoutDesc, std::move(nameToBinding)));
	}
	// build pipeline layout
	PipelineLayoutDesc layoutDesc;
	layoutDesc.setLayouts = std::move(layouts);
	layoutDesc.ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutDesc.ci.pNext = nullptr;
	layoutDesc.ci.flags = 0;
	layoutDesc.ci.pushConstantRangeCount = 0;
	layoutDesc.ci.pPushConstantRanges = nullptr;
	const auto pipelineLayout = device->createPipelineLayout(std::move(layoutDesc));

	VkComputePipelineCreateInfo ci;
	ci.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	ci.pNext = nullptr;
	ci.flags = 0;
	ci.stage = computeShader->getShaderStageCi();
	ci.basePipelineHandle = VK_NULL_HANDLE;
	ci.basePipelineIndex = 0;

	auto pipeline = device->createComputePipeline(std::move(ci), pipelineLayout);

	mtlTemplate = std::make_shared<MaterialTemplateVk>(device, std::vector{computeShader},
													   std::move(layoutNameToIndex), pipeline);
	mtlInstance = mtlTemplate->createInstance();
}
void BasicComputeApp::bindResources(uint32_t swapchainImageIndex)
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

void BasicComputeApp::buildFrameBuffers() {}

void BasicComputeApp::recordCommandBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
										  uint32_t imageIndex)
{
	mtlTemplate->bindPipeline(cmdBuffer);
	mtlInstance->bindDescriptorSets(cmdBuffer);
	cmdBuffer->dispatch(16, 16, 1);
}

void BasicComputeApp::updateResources(std::shared_ptr<VulkanCommandBuffer> cmdBuffer)
{
	imageInfo.width = width;
	imageInfo.height = height;
	imageInfoBuffer->setData(0, &imageInfo, sizeof imageInfo);
}
void BasicComputeApp::render()
{
	syncObjects[currentFrame].submitFence->wait();
	syncObjects[currentFrame].submitFence->reset();

	const auto cmdBuffer = computeCmdBuffers[currentFrame];
	cmdBuffer->reset();
	cmdBuffer->beginCommandBuffer({VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
								   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr});
	updateResources(cmdBuffer);

	const auto imageIndex =
		swapchain->acquireNextImage(syncObjects[currentFrame].imageAcquireComplete, nullptr);
	bindResources(imageIndex);
	recordCommandBuffer(cmdBuffer, imageIndex);
	cmdBuffer->endCommandBuffer();

	SubmitInfoContainer submitData;
	submitData.waitingSemaphores.emplace_back(syncObjects[currentFrame].imageAcquireComplete);
	submitData.waitingStages.emplace_back(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	submitData.signalingSemaphores.emplace_back(syncObjects[currentFrame].renderComplete);
	submitData.signalingFence = syncObjects[currentFrame].submitFence;
	cmdBuffer->submit(submitData);

	VkImageMemoryBarrier barrier;
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	swapchainImages[imageIndex]->image->transitState(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
													 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
													 std::move(barrier));

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
	constexpr int WIDTH = 512;
	constexpr int HEIGHT = 512;
	const char* TITLE = "glfw basic compute test";
	xd::BasicComputeApp app{WIDTH, HEIGHT, TITLE};
	app.init();
	app.run();
	return 0;
}
