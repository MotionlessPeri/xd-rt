//
// Created by Frank on 2024/1/10.
//

#include "VulkanGLFWAppBase.h"
#include <fstream>
#include <ranges>
#include <sstream>

#include "GLFWGlobal.h"
#include "backend/vulkan/FrameGraph.h"
#include "backend/vulkan/VulkanDevice.h"
#include "backend/vulkan/VulkanGlobal.h"
#include "backend/vulkan/VulkanSwapchain.h"

namespace xd {
VulkanGLFWAppBase::VulkanGLFWAppBase(int width, int height, const char* title)
	: width(width), height(height), title(title)
{
}

void VulkanGLFWAppBase::init()
{
	GLFWGlobal::init();
	std::vector<const char*> instanceEnabledExtensions;
	window = createWindow(width, height, title, instanceEnabledExtensions);
	initVulkan(instanceEnabledExtensions);
	loadAssets();

	buildRenderPass();
	buildFrameBuffers();
	createResources();
}

void VulkanGLFWAppBase::run()
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

GLFWwindow* VulkanGLFWAppBase::createWindow(int width,
											int height,
											const char* title,
											std::vector<const char*>& instanceEnableExts)
{
	uint32_t glfwExtensionCnt = 0u;
	auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCnt);

	for ([[maybe_unused]] const auto i : std::views::iota(0u, glfwExtensionCnt)) {
		instanceEnableExts.emplace_back(glfwExtensions[i]);
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	return glfwCreateWindow(width, height, title, nullptr, nullptr);
}

void VulkanGLFWAppBase::initVulkan(const std::vector<const char*>& instanceEnabledExtensions)
{
	VulkanGlobal::init(std::move(instanceEnabledExtensions), {"VK_LAYER_KHRONOS_validation"}, true,
					   glfwGetWin32Window(window), width, height,
					   VkPhysicalDeviceFeatures{.geometryShader = true, .samplerAnisotropy = true},
					   {VK_KHR_SWAPCHAIN_EXTENSION_NAME});
	instance = VulkanGlobal::instance;
	physicalDevice = VulkanGlobal::physicalDevice;
	device = VulkanGlobal::device;
	swapchain = VulkanGlobal::swapchain;
	presentQueue = VulkanGlobal::presentQueue;
	graphicQueue = VulkanGlobal::graphicQueue;
	computeQueue = VulkanGlobal::computeQueue;
	frameCount = swapchain->getSwapchainImageCount();
}

void VulkanGLFWAppBase::handleInput(GLFWwindow* window) {}

}  // namespace xd