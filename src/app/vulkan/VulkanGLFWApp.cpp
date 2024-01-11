//
// Created by Frank on 2024/1/10.
//

#include "VulkanGLFWApp.h"
#include <ranges>
#include "backend/vulkan/VulkanDevice.h"
#include "backend/vulkan/VulkanGlobal.h"
#include "backend/vulkan/VulkanInstance.h"
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

	const auto& instance = VulkanGlobal::instance;
	const auto& physicalDevice = VulkanGlobal::physicalDevice;
	const auto& device = VulkanGlobal::device;

	window = glfwCreateWindow(width, height, title, nullptr, nullptr);

	VulkanGlobal::init(std::move(instanceEnabledExtensions), {"VK_LAYER_KHRONOS_validation"}, true,
					   glfwGetWin32Window(window), width, height, {},
					   {VK_KHR_SWAPCHAIN_EXTENSION_NAME});
}

void VulkanGLFWApp::run()
{
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
	glfwDestroyWindow(window);

	glfwTerminate();
}

}  // namespace xd