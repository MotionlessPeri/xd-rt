//
// Created by Frank on 2024/1/10.
//

#include "VulkanGLFWApp.h"
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ranges>
#include <sstream>
#include <string>
#include "backend/vulkan/FrameGraph.h"
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

namespace xd {
VulkanGLFWApp::VulkanGLFWApp(int width, int height, const char* title)
	: width(width), height(height)
{
	uint32_t glfwExtensionCnt = 0u;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCnt);

	std::vector<const char*> instanceEnabledExtensions{};
	for ([[maybe_unused]] const auto i : std::views::iota(0u, glfwExtensionCnt)) {
		instanceEnabledExtensions.emplace_back(glfwExtensions[i]);
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(width, height, title, nullptr, nullptr);

	VulkanGlobal::init(std::move(instanceEnabledExtensions), {"VK_LAYER_KHRONOS_validation"}, true,
					   glfwGetWin32Window(window), width, height,
					   VkPhysicalDeviceFeatures{.geometryShader = true, .samplerAnisotropy = true},
					   {VK_KHR_SWAPCHAIN_EXTENSION_NAME});
	instance = VulkanGlobal::instance;
	physicalDevice = VulkanGlobal::physicalDevice;
	device = VulkanGlobal::device;
	swapchain = VulkanGlobal::swapchain;
	presentQueue = VulkanGlobal::presentQueue;
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
	loadAssets();
	createResources();
	buildDescriptors();
	buildRenderPass();
	buildPipeline();
	buildFrameBuffers();
}

void VulkanGLFWApp::run()
{
	while (!glfwWindowShouldClose(window)) {
		const auto timeStart = std::chrono::steady_clock::now();
		glfwPollEvents();
		draw();
		const auto timeEnd = std::chrono::steady_clock::now();
		elapsedTime = std::chrono::duration<float>{timeEnd - timeStart}.count();
	}
	device->waitIdle();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void VulkanGLFWApp::handleInput(GLFWwindow* window) {}

void VulkanGLFWApp::loadAssets()
{
	ObjMeshLoader loader;
	auto meshWithNoAccel = loader.load(R"(D:\qem-test.obj)");
	model = ModelFactoryVk::get().buildTriangleMesh(meshWithNoAccel);

	const auto cpuImage = TextureFactory::get().loadUVTexture(R"(D:\uv_checker.jpg)");
	texture = TextureFactoryVk::get().buildTexture(cpuImage);
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
}

void VulkanGLFWApp::createResources()
{
	uniformData.model = glm::mat4{1.f};
	uniformData.view = glm::lookAt(glm::vec3{2, 0, 0}, glm::vec3{0, 0, 0}, -glm::vec3{0, 1, 0});
	uniformData.proj = glm::perspective(90.f, (float)width / height, 0.1f, 100.f);
	uniformData.normalTransform = glm::transpose(glm::inverse(uniformData.model));

	VkBufferCreateInfo bufferCi;
	bufferCi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCi.pNext = nullptr;
	bufferCi.flags = 0;
	bufferCi.size = sizeof(uniformData);
	bufferCi.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferCi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	uniformBuffer = device->createBuffer(
		bufferCi, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

void VulkanGLFWApp::buildRenderPass()
{
	FrameGraphBuilder builder;
	auto& subpass = builder.addSubpass("main pass");
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
	renderPass = frameGraph->renderPass;
}

void VulkanGLFWApp::buildDescriptors()
{
	DescriptorPoolDesc poolDesc;
	poolDesc.poolSizes = std::vector<VkDescriptorPoolSize>{
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
	};
	poolDesc.ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolDesc.ci.pNext = nullptr;
	poolDesc.ci.flags = 0;
	poolDesc.ci.maxSets = frameCount;
	poolDesc.ci.poolSizeCount = poolDesc.poolSizes.size();
	poolDesc.ci.pPoolSizes = poolDesc.poolSizes.data();
	descPool = device->createDescriptorPool(poolDesc);
	DescriptorSetLayoutDesc layoutDesc;
	layoutDesc.bindings = std::vector<VkDescriptorSetLayoutBinding>{
		{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};
	layoutDesc.ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutDesc.ci.pNext = nullptr;
	layoutDesc.ci.flags = 0;
	layoutDesc.ci.bindingCount = layoutDesc.bindings.size();
	layoutDesc.ci.pBindings = layoutDesc.bindings.data();
	descSetLayout = device->createDescriptorSetLayout(layoutDesc);
	frameResources.resize(frameCount);
	for (auto& resource : frameResources) {
		resource.descSet = descSetLayout->createDescriptorSet(descPool);
	}
}

void VulkanGLFWApp::bindResources()
{
	const auto& descSet = frameResources[currentFrame].descSet;
	descSet->bindResource(0, uniformBuffer->getBindingInfo());
	descSet->bindResource(1, texture->getBindingInfo(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
	descSet->updateDescriptors();
}

void VulkanGLFWApp::buildPipeline()
{
	GraphicsPipelineDesc pipelineDesc;
	// build pipeline layout
	pipelineDesc.layoutCi.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineDesc.layoutCi.pNext = nullptr;
	pipelineDesc.layoutCi.flags = 0;
	pipelineDesc.layoutCi.setLayoutCount = 1;
	pipelineDesc.layoutCi.pSetLayouts = &descSetLayout->layout;
	pipelineDesc.layoutCi.pushConstantRangeCount = 0;	  // Optional
	pipelineDesc.layoutCi.pPushConstantRanges = nullptr;  // Optional
	std::shared_ptr<VulkanShader> vs, fs;
	using namespace std::string_literals;
	{
		// std::istringstream stream{"./shader/test.vert"};
		std::ifstream fstream{PROJECT_ROOT + "/vulkan/shader/test.vert.spv"s,
							  std::ios::ate | std::ios::binary};
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
		vs = device->createShader(ci, VK_SHADER_STAGE_VERTEX_BIT);
		shaders.emplace_back(vs);
	}
	{
		std::ifstream fstream{PROJECT_ROOT + "/vulkan/shader/test.frag.spv"s,
							  std::ios::ate | std::ios::binary};
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
		fs = device->createShader(ci, VK_SHADER_STAGE_FRAGMENT_BIT);
		shaders.emplace_back(fs);
	}

	pipelineDesc.dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(pipelineDesc.dynamicStates.size());
	dynamicState.pDynamicStates = pipelineDesc.dynamicStates.data();

	const auto vertexInputInfo = ModelFactoryVk::get().getVertexInputState();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// dynamic viewport and scissor state
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;	// Optional
	rasterizer.depthBiasClamp = 0.0f;			// Optional
	rasterizer.depthBiasSlopeFactor = 0.0f;		// Optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;			 // Optional
	multisampling.pSampleMask = nullptr;			 // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE;	 // Optional
	multisampling.alphaToOneEnable = VK_FALSE;		 // Optional

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;	 // Optional
	depthStencil.maxDepthBounds = 1.0f;	 // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};  // Optional
	depthStencil.back = {};	  // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
										  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	// over op
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	pipelineDesc.colorBlendStates.emplace_back(colorBlendAttachment);

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;  // Optional
	colorBlending.attachmentCount = pipelineDesc.colorBlendStates.size();
	colorBlending.pAttachments = pipelineDesc.colorBlendStates.data();

	pipelineDesc.shaderStages = {vs->getShaderStageCi(), fs->getShaderStageCi()};

	pipelineDesc.ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineDesc.ci.pNext = nullptr;
	pipelineDesc.ci.flags = 0;
	pipelineDesc.ci.stageCount = pipelineDesc.shaderStages.size();
	pipelineDesc.ci.pStages = pipelineDesc.shaderStages.data();

	pipelineDesc.ci.pVertexInputState = &vertexInputInfo.ci;
	pipelineDesc.ci.pInputAssemblyState = &inputAssembly;
	pipelineDesc.ci.pViewportState = &viewportState;
	pipelineDesc.ci.pRasterizationState = &rasterizer;
	pipelineDesc.ci.pMultisampleState = &multisampling;
	pipelineDesc.ci.pDepthStencilState = &depthStencil;	 // Optional
	pipelineDesc.ci.pColorBlendState = &colorBlending;
	pipelineDesc.ci.pDynamicState = &dynamicState;

	pipeline = renderPass->getSubpasses().front()->createGraphicsPipeline(std::move(pipelineDesc));
}

void VulkanGLFWApp::buildFrameBuffers()
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

void VulkanGLFWApp::recordCommandBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
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
		info.renderPass = renderPass->pass;
		info.framebuffer = frameBuffers[imageIndex]->frameBuffer;
		info.renderArea.offset = {0, 0};
		info.renderArea.extent = swapchainExtent;
		info.clearValueCount = clearValues.size();
		info.pClearValues = clearValues.data();
		cmdBuffer->beginRenderPass(info, VK_SUBPASS_CONTENTS_INLINE);
	}
	cmdBuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
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

	pipeline->bindDescriptorSets(cmdBuffer, 0, {frameResources[currentFrame].descSet});

	model->bind(cmdBuffer);
	model->draw(cmdBuffer);

	cmdBuffer->endRenderPass();
}

void VulkanGLFWApp::updateResources(std::shared_ptr<VulkanCommandBuffer> cmdBuffer)
{
	uniformBuffer->setData(0, &uniformData, sizeof(uniformData));
}

void VulkanGLFWApp::draw()
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