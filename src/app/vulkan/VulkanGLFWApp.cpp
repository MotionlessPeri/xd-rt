//
// Created by Frank on 2024/1/10.
//

#include "VulkanGLFWApp.h"
#include <fstream>
#include <ranges>
#include <sstream>
#include <string>

#include "backend/vulkan/ModelFactoryVk.h"
#include "backend/vulkan/TriangleMeshVk.h"
#include "backend/vulkan/VulkanCommandBuffer.h"
#include "backend/vulkan/VulkanCommandPool.h"
#include "backend/vulkan/VulkanDevice.h"
#include "backend/vulkan/VulkanFence.h"
#include "backend/vulkan/VulkanFrameBuffer.h"
#include "backend/vulkan/VulkanGlobal.h"
#include "backend/vulkan/VulkanGraphicsPipeline.h"
#include "backend/vulkan/VulkanImageView.h"
#include "backend/vulkan/VulkanQueue.h"
#include "backend/vulkan/VulkanRenderPass.h"
#include "backend/vulkan/VulkanShader.h"
#include "backend/vulkan/VulkanSwapchain.h"
#include "loader/ObjMeshLoader.h"

namespace xd {
VulkanGLFWApp::VulkanGLFWApp(int width, int height, const char* title)
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
					   glfwGetWin32Window(window), width, height, {},
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
	pool = device->createCommandPool(std::move(poolCi));
	VkCommandBufferAllocateInfo cmdBufferInfo;
	cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufferInfo.pNext = nullptr;
	cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferInfo.commandBufferCount = frameCount;
	cmdBuffers = pool->allocateCommandBuffers(std::move(cmdBufferInfo));
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
	buildRenderPass();
	buildPipeline();
	buildFrameBuffers();
	for ([[maybe_unused]] const auto i : std::views::iota(0u, frameCount)) {
		recordCommandBuffer(i);
	}
}

void VulkanGLFWApp::run()
{
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		draw();
	}
	device->waitIdle();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void VulkanGLFWApp::loadAssets()
{
	ObjMeshLoader loader;
	auto meshWithNoAccel = loader.load(R"(D:\qem-test.obj)");
	model = ModelFactoryVk::get().buildTriangleMesh(meshWithNoAccel);
}

void VulkanGLFWApp::buildRenderPass()
{
	RenderPassDesc desc;
	// build attachment desc
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
	desc.attachments.emplace_back(colorAttachment);

	SubpassDesc subpassDesc;
	// build attachment ref
	VkAttachmentReference2 colorAttachmentRef{};
	colorAttachmentRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
	colorAttachmentRef.pNext = nullptr;
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachmentRef.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subpassDesc.colorAttaches.emplace_back(colorAttachmentRef);
	VkAttachmentReference2 depthStencilRef{};
	depthStencilRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
	depthStencilRef.pNext = nullptr;
	depthStencilRef.attachment = VK_ATTACHMENT_UNUSED;
	depthStencilRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	depthStencilRef.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	VkAttachmentReference2 resolveRef{};
	resolveRef.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
	resolveRef.pNext = nullptr;
	resolveRef.attachment = VK_ATTACHMENT_UNUSED;
	resolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	resolveRef.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	// build subpass
	VkSubpassDescription2 subpass{};
	subpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
	subpass.pNext = nullptr;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.viewMask = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pResolveAttachments = nullptr;
	subpass.pDepthStencilAttachment = nullptr;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;
	desc.subpasses.emplace_back(subpass);
	// build dependencies
	VkSubpassDependency2 dependency{};
	dependency.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
	dependency.pNext = nullptr;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = 0;
	dependency.viewOffset = 0;
	// build render pass
	desc.ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
	desc.ci.pNext = nullptr;
	desc.ci.flags = 0;
	desc.ci.attachmentCount = 1;
	desc.ci.pAttachments = &colorAttachment;
	desc.ci.subpassCount = 1;
	desc.ci.pSubpasses = &subpass;
	desc.ci.dependencyCount = 1;
	desc.ci.pDependencies = &dependency;
	desc.ci.correlatedViewMaskCount = 0;
	desc.ci.pCorrelatedViewMasks = nullptr;
	renderPass = device->createRenderPass(desc);
}

void VulkanGLFWApp::buildPipeline()
{
	GraphicsPipelineDesc pipelineDesc;
	// build pipeline layout
	pipelineDesc.layoutCi.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineDesc.layoutCi.pNext = nullptr;
	pipelineDesc.layoutCi.flags = 0;
	pipelineDesc.layoutCi.setLayoutCount = 0;			  // Optional
	pipelineDesc.layoutCi.pSetLayouts = nullptr;		  // Optional
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

	// const auto swapchainExtent = VulkanGlobal::swapchain->getExtent();
	// VkViewport viewport{};
	// viewport.x = 0.0f;
	// viewport.y = 0.0f;
	// viewport.width = (float)swapchainExtent.width;
	// viewport.height = (float)swapchainExtent.height;
	// viewport.minDepth = 0.0f;
	// viewport.maxDepth = 1.0f;
	// VkRect2D scissor{};
	// scissor.offset = {0, 0};
	// scissor.extent = swapchainExtent;

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
	pipelineDesc.ci.pDepthStencilState = nullptr;  // Optional
	pipelineDesc.ci.pColorBlendState = &colorBlending;
	pipelineDesc.ci.pDynamicState = &dynamicState;

	pipeline = renderPass->getSubpasses().front()->createGraphicsPipeline(std::move(pipelineDesc));
}

void VulkanGLFWApp::buildFrameBuffers()
{
	const auto& swapchainImages = swapchain->getSwapchainImages();
	const auto swapchainExtent = swapchain->getExtent();
	const auto subpass = renderPass->getSubpasses().front();
	frameBuffers.resize(swapchainImages.size());
	std::ranges::transform(swapchainImages, frameBuffers.begin(), [&](const auto& swapchainImage) {
		VkFramebufferCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		ci.attachmentCount = 1;
		ci.pAttachments = &swapchainImage.view->imageView;
		ci.width = swapchainExtent.width;
		ci.height = swapchainExtent.height;
		ci.layers = 1;
		return subpass->createFrameBuffer(std::move(ci));
	});
}

void VulkanGLFWApp::recordCommandBuffer(uint32_t imageIndex)
{
	const auto cmdBuffer = cmdBuffers[currentFrame];
	cmdBuffer->reset();
	cmdBuffer->beginCommandBuffer({VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr,
								   VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr});
	const auto swapchainExtent = swapchain->getExtent();
	{
		VkRenderPassBeginInfo info;
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.pNext = nullptr;
		info.renderPass = renderPass->pass;
		info.framebuffer = frameBuffers[imageIndex]->frameBuffer;
		info.renderArea.offset = {0, 0};
		info.renderArea.extent = swapchainExtent;
		VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
		info.clearValueCount = 1;
		info.pClearValues = &clearColor;
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

	model->bind(cmdBuffer);
	model->draw(cmdBuffer);

	cmdBuffer->endRenderPass();
	cmdBuffer->endCommandBuffer();
}

void VulkanGLFWApp::draw()
{
	// waiting for fence before using it
	syncObjects[currentFrame].submitFence->wait();
	syncObjects[currentFrame].submitFence->reset();

	const auto imageIndex =
		swapchain->acquireNextImage(syncObjects[currentFrame].imageAcquireComplete, nullptr);
	recordCommandBuffer(imageIndex);

	SubmitInfoContainer submitData;
	submitData.waitingSemaphores.emplace_back(syncObjects[currentFrame].imageAcquireComplete);
	submitData.waitingStages.emplace_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	submitData.signalingSemaphores.emplace_back(syncObjects[currentFrame].renderComplete);
	submitData.signalingFence = syncObjects[currentFrame].submitFence;
	cmdBuffers[currentFrame]->submit(submitData);

	QueuePresentInfoContainer desc;
	desc.waitSemaphores.emplace_back(syncObjects[currentFrame].renderComplete);
	desc.swapchains.emplace_back(swapchain);
	desc.imageIndices.emplace_back(imageIndex);
	presentQueue->present(desc);
	currentFrame = (currentFrame + 1) % frameCount;
}
}  // namespace xd