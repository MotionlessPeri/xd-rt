//
// Created by Frank on 2024/2/23.
//

#include "BasicFramegraphApp.h"

#include <glm/gtc/matrix_transform.hpp>

#include "backend/vulkan/FrameGraph.h"
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
#include "backend/vulkan/VulkanGlobal.h"
#include "backend/vulkan/VulkanImage.h"
#include "backend/vulkan/VulkanInstance.h"
#include "backend/vulkan/VulkanPhysicalDevice.h"
#include "backend/vulkan/VulkanQueue.h"
#include "backend/vulkan/VulkanRenderPass.h"
#include "backend/vulkan/VulkanSwapchain.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"

#include "loader/TextureFactory.h"
#include "model/Sphere.h"

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}
namespace xd {
BasicFramegraphApp::BasicFramegraphApp(int width, int height, const char* title)
	: VulkanGLFWAppBase(width, height, title)
{
}

void BasicFramegraphApp::initVulkan(const std::vector<const char*>& instanceEnabledExtensions)
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

void BasicFramegraphApp::loadAssets()
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

void BasicFramegraphApp::createResources()
{
	// create swapchain images
	const auto& rawOutputImages = swapchain->getSwapchainImages();
	swapchainImages.resize(frameCount);
	std::ranges::transform(rawOutputImages, swapchainImages.begin(),
						   [](const auto& swapchainImage) {
							   auto ret = std::make_shared<TextureVk>();
							   ret->image = swapchainImage.image;
							   ret->imageView = swapchainImage.view;
							   ret->sampler = nullptr;
							   return ret;
						   });
	// create cmd pools
	{
		VkCommandPoolCreateInfo poolCi;
		poolCi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolCi.pNext = nullptr;
		poolCi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolCi.queueFamilyIndex = VulkanGlobal::graphicQueue->getQueueFamilyIndex();
		graphicCmdPool = device->createCommandPool(std::move(poolCi));
	}

	{
		VkCommandPoolCreateInfo poolCi;
		poolCi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolCi.pNext = nullptr;
		poolCi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolCi.queueFamilyIndex = computeQueue->getQueueFamilyIndex();
		computeCmdPool = device->createCommandPool(std::move(poolCi));
	}
	// create frame graph
	{
		auto fgBuilder = FGBuilder{device};
		auto& colorPass = fgBuilder.addGraphicsPass("ColorPass");
		auto& colorSubpass = colorPass.addSubpass(
			"ColorSubpass",
			[&](FGResourceList& resources, std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
				const std::vector<std::shared_ptr<VulkanSemaphore>>& waitingSemaphores) {
				const auto swapchainExtent = swapchain->getExtent();
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
			});
		VkImageCreateInfo imageCi;
		imageCi.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCi.pNext = nullptr;
		imageCi.flags = 0;
		imageCi.imageType = VK_IMAGE_TYPE_2D;
		imageCi.format = swapchain->getFormat().format;
		imageCi.extent.width = swapchain->getExtent().width;
		imageCi.extent.height = swapchain->getExtent().height;
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
		VkImageViewCreateInfo imageViewCi;
		imageViewCi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCi.pNext = nullptr;
		imageViewCi.flags = 0;
		imageViewCi.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCi.format = imageCi.format;
		imageViewCi.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCi.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCi.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCi.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		imageViewCi.subresourceRange.baseMipLevel = 0;
		imageViewCi.subresourceRange.levelCount = 1;
		imageViewCi.subresourceRange.baseArrayLayer = 0;
		imageViewCi.subresourceRange.layerCount = 1;
		auto colorImageDummy = colorSubpass.importImage(imageCi, imageViewCi, nullptr);
		auto* colorAttach =
			colorImageDummy.createColorAttach("color", colorSubpass, VK_ATTACHMENT_LOAD_OP_CLEAR,
											  VK_ATTACHMENT_STORE_OP_STORE, {0, 0, 0, 0});
		// create depth attach
		{
			const auto swapchainExtent = swapchain->getExtent();
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
			colorSubpass.createImage(imageCi, viewCi, nullptr)
				.createDepthAttach("depth", colorSubpass,
								   VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
									   VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
									   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
								   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
									   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
								   VK_IMAGE_ASPECT_DEPTH_BIT, VK_ATTACHMENT_LOAD_OP_CLEAR,
								   VK_ATTACHMENT_STORE_OP_DONT_CARE,
								   VK_ATTACHMENT_LOAD_OP_DONT_CARE,
								   VK_ATTACHMENT_STORE_OP_DONT_CARE, {1.f, 0});
		}
		auto& tonemappingPass = fgBuilder.addComputePass(
			"TonemappingPass",
			[&](FGResourceList& resources, std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
				const std::vector<std::shared_ptr<VulkanSemaphore>>& waitingSemaphores) {
				const auto outputTexture = resources.getImage("TonemappingPass", "input");

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
				tonemappingMtl.mtlTemplate->bindPipeline(cmdBuffer);
				tonemappingMtl.mtlInstance->bindDescriptorSets(cmdBuffer);
				cmdBuffer->dispatch(16, 16, 1);
			});
		colorAttach->createImageBinding(
			"input", tonemappingPass, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT);
		auto* tonemapped = colorAttach->createImageBinding(
			"output", tonemappingPass, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_ACCESS_SHADER_WRITE_BIT);
		auto& imguiPass = fgBuilder.addGraphicsPass("ImguiPass");
		auto& imguiSubpass = imguiPass.addSubpass(
			"ImguiSubpass",
			[&](FGResourceList& resources, std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
				const std::vector<std::shared_ptr<VulkanSemaphore>>& waitingSemaphores) {
				ImGui_ImplVulkan_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				ImGui::Begin("Hello World");
				ImGui::Checkbox("Enable Tonemapping", &enableTonemapping);
				ImGui::End();
				ImGui::Render();
				ImDrawData* imguiDrawData = ImGui::GetDrawData();
				ImGui_ImplVulkan_RenderDrawData(imguiDrawData, cmdBuffer->cmdBuffer);
			});
		auto* withGui = tonemapped->createColorAttach(
			"color", imguiSubpass, VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE);
		auto& presentPass = fgBuilder.addPass(
			"PresentPass", PassType::OTHER,
			[&](FGResourceList& resources, std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
				const std::vector<std::shared_ptr<VulkanSemaphore>>& waitingSemaphores) {
				QueuePresentInfoContainer desc;
				desc.swapchains.emplace_back(swapchain);
				desc.imageIndices.emplace_back(currentImageIndex);
				desc.waitSemaphores = waitingSemaphores;
				presentQueue->present(desc);
			},
			true);
		withGui->createImageBinding("present", presentPass, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
									VK_IMAGE_ASPECT_NONE, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
									VK_ACCESS_MEMORY_READ_BIT);
		frameGraph = fgBuilder.build(device);
	}
	// create material resources
	{
		auto& mtlFactory = MaterialFactoryVk::get();
		lambertianMtl.mtlTemplate = mtlFactory.createLambertianMaterial(
			frameGraph->getSubpass("ColorSubpass").getSubpass());
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
		// bind resource to material instance
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
	// create FGResources
	{
		const int swapchainImageCount = swapchainImages.size();
		resourceLists.resize(swapchainImageCount);
		for (const auto i : std::views::iota(0, swapchainImageCount)) {
			resourceLists[i] =
				frameGraph->createResourceList(graphicCmdPool, computeCmdPool, nullptr);
		}
	}
	// init imgui
	{
		// desc pool for imgui
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
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

		// TODO: integrate ImGui to xd_rt
		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = instance->instance;
		init_info.PhysicalDevice = physicalDevice->device;
		init_info.Device = device->device;
		init_info.QueueFamily = physicalDevice->getSuitableQueueFamilyIndex(
			[](VkPhysicalDevice device, const VkQueueFamilyProperties& properties,
			   int index) -> bool { return properties.queueFlags & (VK_QUEUE_GRAPHICS_BIT); });

		init_info.Queue = graphicQueue->queue;
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = descPool->pool;
		init_info.Subpass = 0;
		init_info.MinImageCount = 2;
		init_info.ImageCount = swapchain->getSwapchainImageCount();
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = check_vk_result;
		ImGui_ImplVulkan_Init(&init_info,
							  frameGraph->getSubpass("ImguiSubpass").getRenderPass()->pass);
	}
}

void BasicFramegraphApp::updateResources(std::shared_ptr<VulkanCommandBuffer> cmdBuffer)
{
	imageInfo.width = width;
	imageInfo.height = height;
	imageInfo.enableTonemapping = enableTonemapping ? 1 : 0;
	imageInfoBuffer->setData(0, &imageInfo, sizeof imageInfo);
	uniformBuffer->setData(0, &uniformData, sizeof uniformData);
}

void BasicFramegraphApp::render()
{
	updateResources(nullptr);
	auto resourceList = resourceLists[currentFrame];
	std::shared_ptr<VulkanSemaphore> semaphore = nullptr;
	if (resourceList->hasValue("imageAcquireSemaphore")) {
		semaphore =
			resourceList->getValue<std::shared_ptr<VulkanSemaphore>>("imageAcquireSemaphore");
	}
	else {
		VkSemaphoreCreateInfo semaphoreCi;
		semaphoreCi.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCi.pNext = nullptr;
		semaphoreCi.flags = 0;
		semaphore = device->createSemaphore(semaphoreCi);
		resourceList->storeValue("imageAcquireSemaphore", semaphore);
		resourceList->addExternalWaitingSemaphore("ColorSubpass", semaphore,
												  VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	}

	currentImageIndex = swapchain->acquireNextImage(semaphore, nullptr);

	resourceList->bindImage("PresentPass", "present", swapchainImages[currentImageIndex],
							VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
							VK_ACCESS_MEMORY_READ_BIT);

	frameGraph->execute(resourceLists[currentFrame]);
	currentFrame = (currentFrame + 1) % frameCount;
}

void BasicFramegraphApp::buildPipeline() {}
void BasicFramegraphApp::buildFrameBuffers() {}

void BasicFramegraphApp::recordCommandBuffer(std::shared_ptr<VulkanCommandBuffer> cmdBuffer,
											 uint32_t imageIndex)
{
}
}  // namespace xd
int main()
{
	constexpr int WIDTH = 1000;
	constexpr int HEIGHT = 800;
	const char* TITLE = "glfw basic compute test";
	xd::BasicFramegraphApp app{WIDTH, HEIGHT, TITLE};
	app.init();
	app.run();
	return 0;
}