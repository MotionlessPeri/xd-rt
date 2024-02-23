//
// Created by Frank on 2024/2/22.
//

#include "ImguiAppBase.h"

#include "backend/vulkan/VulkanCommandBuffer.h"
#include "backend/vulkan/VulkanCommandPool.h"
#include "backend/vulkan/VulkanDevice.h"
#include "backend/vulkan/VulkanFence.h"
#include "backend/vulkan/VulkanFrameBuffer.h"
#include "backend/vulkan/VulkanGlobal.h"
#include "backend/vulkan/VulkanImageView.h"
#include "backend/vulkan/VulkanInstance.h"
#include "backend/vulkan/VulkanQueue.h"
#include "backend/vulkan/VulkanRenderPass.h"
#include "backend/vulkan/VulkanSwapchain.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of
// testing and compatibility with old VS compilers. To link with VS2010-era libraries, VS2015+
// requires linking with legacy_stdio_definitions.lib, which we do using this pragma. Your own
// project should not be affected, as you are likely to link with a newer binary of GLFW that is
// adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// #define IMGUI_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif
static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

namespace xd {
void ImguiAppBase::init()
{
	VulkanGLFWAppBase::init();
	initImgui();
}

void ImguiAppBase::run()
{
	while (!glfwWindowShouldClose(window)) {
		const auto timeStart = std::chrono::steady_clock::now();
		glfwPollEvents();
		render();
		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		renderImgui();
		present();

		const auto timeEnd = std::chrono::steady_clock::now();
		elapsedTime = std::chrono::duration<float>{timeEnd - timeStart}.count();
	}
	device->waitIdle();

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

ImguiAppBase::ImguiAppBase(int width, int height, const char* title)
	: VulkanGLFWAppBase(width, height, title)
{
}

void ImguiAppBase::initVulkan(const std::vector<const char*>& instanceEnabledExtensions)
{
	VulkanGlobal::init(std::move(instanceEnabledExtensions), {"VK_LAYER_KHRONOS_validation"}, true,
					   glfwGetWin32Window(window), width, height,
					   VkPhysicalDeviceFeatures{.geometryShader = true, .samplerAnisotropy = true},
					   {VK_KHR_SWAPCHAIN_EXTENSION_NAME},
					   {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
					   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
	instance = VulkanGlobal::instance;
	physicalDevice = VulkanGlobal::physicalDevice;
	device = VulkanGlobal::device;
	swapchain = VulkanGlobal::swapchain;
	presentQueue = VulkanGlobal::presentQueue;
	graphicQueue = VulkanGlobal::graphicQueue;
	computeQueue = VulkanGlobal::computeQueue;
	frameCount = swapchain->getSwapchainImageCount();
}

void ImguiAppBase::initImgui()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

	// allocate command buffers
	{
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
	}

	{
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
	}

	{
		VkCommandPoolCreateInfo poolCi;
		poolCi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolCi.pNext = nullptr;
		poolCi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolCi.queueFamilyIndex = graphicQueue->getQueueFamilyIndex();
		imguiCmdPool = device->createCommandPool(std::move(poolCi));
		VkCommandBufferAllocateInfo cmdBufferInfo;
		cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferInfo.pNext = nullptr;
		cmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferInfo.commandBufferCount = frameCount;
		imguiCmdBuffers = imguiCmdPool->allocateCommandBuffers(std::move(cmdBufferInfo));
	}
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
		imguiSyncObjects.emplace_back(device->createSemaphore(semaphoreCi),
									  device->createSemaphore(semaphoreCi),
									  device->createFence(fenceCi));
	}

	DescriptorPoolDesc descPoolDesc;
	descPoolDesc.poolSizes = {
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
	};
	descPoolDesc.ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolDesc.ci.pNext = nullptr;
	descPoolDesc.ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descPoolDesc.ci.maxSets = 1;
	descPoolDesc.ci.poolSizeCount = descPoolDesc.poolSizes.size();
	descPoolDesc.ci.pPoolSizes = descPoolDesc.poolSizes.data();
	descPool = device->createDescriptorPool(descPoolDesc);

	{
		RenderPassDesc desc;
		VkAttachmentDescription2 attachment = {};
		attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
		attachment.pNext = nullptr;
		attachment.format = swapchain->getFormat().format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		desc.attachments.emplace_back(attachment);

		VkAttachmentReference2 color_attachment = {};
		color_attachment.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
		color_attachment.pNext = nullptr;
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		SubpassDesc subpass = {};
		subpass.colorRefs.emplace_back(color_attachment);
		subpass.desc.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
		subpass.desc.pNext = nullptr;
		subpass.desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.desc.colorAttachmentCount = subpass.colorRefs.size();
		subpass.desc.pColorAttachments = subpass.colorRefs.data();
		desc.subpasses.emplace_back(subpass);

		VkSubpassDependency2 dependency = {};
		dependency.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
		dependency.pNext = nullptr;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		desc.dependencys.emplace_back(dependency);

		std::vector<VkSubpassDescription2> vkSubpasses;
		vkSubpasses.emplace_back(subpass.desc);

		desc.ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
		desc.ci.pNext = nullptr;
		desc.ci.flags = 0;
		desc.ci.attachmentCount = desc.attachments.size();
		desc.ci.pAttachments = desc.attachments.data();
		desc.ci.subpassCount = vkSubpasses.size();
		desc.ci.pSubpasses = vkSubpasses.data();
		desc.ci.dependencyCount = desc.dependencys.size();
		desc.ci.pDependencies = desc.dependencys.data();
		desc.ci.correlatedViewMaskCount = 0;
		desc.ci.pCorrelatedViewMasks = nullptr;
		imguiRenderPass = device->createRenderPass(desc);
	}

	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = instance->instance;
	init_info.PhysicalDevice = physicalDevice->device;
	init_info.Device = device->device;
	init_info.QueueFamily = physicalDevice->getSuitableQueueFamilyIndex(
		[](VkPhysicalDevice device, const VkQueueFamilyProperties& properties, int index) -> bool {
			return properties.queueFlags & (VK_QUEUE_GRAPHICS_BIT);
		});

	init_info.Queue = graphicQueue->queue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = descPool->pool;
	init_info.Subpass = 0;
	init_info.MinImageCount = 2;
	init_info.ImageCount = swapchain->getSwapchainImageCount();
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = nullptr;
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info, imguiRenderPass->pass);

	imguiFrameBuffers.resize(frameCount);
	const auto& swapchainImages = swapchain->getSwapchainImages();
	for (const auto i : std::views::iota(0ull, swapchainImages.size())) {
		const auto& image = swapchainImages[i];
		VkFramebufferCreateInfo ci;
		ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = 0;
		ci.renderPass = imguiRenderPass->pass;
		ci.attachmentCount = 1;
		ci.pAttachments = &image.view->imageView;
		ci.width = width;
		ci.height = height;
		ci.layers = 1;
		imguiFrameBuffers[i] = imguiRenderPass->createFrameBuffer(std::move(ci));
	}
}

void ImguiAppBase::renderImgui()
{
	imguiSyncObjects[currentFrame].submitFence->wait();
	imguiSyncObjects[currentFrame].submitFence->reset();

	currentImageIndex =
		swapchain->acquireNextImage(imguiSyncObjects[currentFrame].imageAcquireComplete, nullptr);
	auto cmdBuffer = graphicCmdBuffers[currentFrame];
	recordImguiCmdBuffer(cmdBuffer);

	SubmitInfoContainer submitData;
	submitData.waitingSemaphores.emplace_back(imguiSyncObjects[currentFrame].imageAcquireComplete);
	submitData.waitingStages.emplace_back(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
	submitData.signalingSemaphores.emplace_back(imguiSyncObjects[currentFrame].renderComplete);
	submitData.signalingFence = imguiSyncObjects[currentFrame].submitFence;
	cmdBuffer->submit(submitData);
}

void ImguiAppBase::recordImguiCmdBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer)
{
	ImGui::Render();
	ImDrawData* imguiDrawData = ImGui::GetDrawData();

	VkRenderPassBeginInfo info;
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	info.pNext = nullptr;
	info.renderPass = imguiRenderPass->pass;
	info.framebuffer = imguiFrameBuffers[currentImageIndex]->frameBuffer;
	info.renderArea.offset = {0, 0};
	info.renderArea.extent = swapchain->getExtent();
	info.clearValueCount = 0;
	info.pClearValues = nullptr;
	cmdBuffer->beginRenderPass(info, VK_SUBPASS_CONTENTS_INLINE);

	ImGui_ImplVulkan_RenderDrawData(imguiDrawData, cmdBuffer->cmdBuffer);

	cmdBuffer->endRenderPass();
}

void ImguiAppBase::present()
{
	QueuePresentInfoContainer desc;
	desc.waitSemaphores.emplace_back(imguiSyncObjects[currentFrame].renderComplete);
	desc.swapchains.emplace_back(swapchain);
	desc.imageIndices.emplace_back(currentImageIndex);
	presentQueue->present(desc);
	currentFrame = (currentFrame + 1) % frameCount;
}
}  // namespace xd
