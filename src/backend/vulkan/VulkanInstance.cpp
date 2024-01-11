//
// Created by Frank on 2024/1/9.
//

#include "VulkanInstance.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include "VulkanMacros.h"
#include "VulkanSurface.h"
using namespace xd;

#ifdef _DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			  VkDebugUtilsMessageTypeFlagsEXT messageType,
			  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			  void* pUserData)
{
	std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;
	// std::abort();
	__debugbreak();
	return VK_FALSE;
}
#endif
VulkanInstance::VulkanInstance(std::vector<const char*>&& enabledExtensions,
							   std::vector<const char*>&& enabledLayers)
{
	uint32_t extensionCount = 0;
	CHECK_VK_ERROR(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
	std::vector<VkExtensionProperties> supportedExts(extensionCount);
	CHECK_VK_ERROR(
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, supportedExts.data()));
	std::unordered_set<std::string> supportedInstanceExtensionsSet;
	for (const auto& ext : supportedExts) {
		supportedInstanceExtensionsSet.emplace(ext.extensionName);
	}

	uint32_t layerCount = 0;
	CHECK_VK_ERROR(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));
	std::vector<VkLayerProperties> supportedLayers(layerCount);
	CHECK_VK_ERROR(vkEnumerateInstanceLayerProperties(&layerCount, supportedLayers.data()));
	std::unordered_set<std::string> supportedLayersSet;
	for (const auto& layer : supportedLayers) {
		supportedLayersSet.emplace(layer.layerName);
	}

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "XD";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

#ifdef _DEBUG
	enabledExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
	for (const auto* extName : enabledExtensions) {
		if (supportedInstanceExtensionsSet.count(extName) == 0) {
			throw std::runtime_error{""};
		}
	}

	for (const auto* layerName : enabledLayers) {
		if (supportedLayersSet.count(layerName) == 0) {
			throw std::runtime_error{""};
		}
	}

	createInfo.enabledExtensionCount = enabledExtensions.size();
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();
	createInfo.enabledLayerCount = enabledLayers.size();
	createInfo.ppEnabledLayerNames = enabledLayers.data();

	CHECK_VK_ERROR(vkCreateInstance(&createInfo, nullptr, &instance));

	// assign debug messanger
#ifdef _DEBUG
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
							 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
							 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;	 // Optional
	const auto CreateDebugUtilsMessengerEXT =
		[](VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		   const VkAllocationCallbacks* pAllocator,
		   VkDebugUtilsMessengerEXT* pDebugMessenger) -> VkResult {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	};
	CHECK_VK_ERROR(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger));
#endif
}

VulkanInstance::~VulkanInstance()
{
#ifdef _DEBUG
	const auto DestroyDebugUtilsMessengerEXT = [](VkInstance instance,
												  VkDebugUtilsMessengerEXT debugMessenger,
												  const VkAllocationCallbacks* pAllocator) -> void {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	};
	DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif
	vkDestroyInstance(instance, nullptr);
}

VkSurfaceKHR VulkanInstance::createRawSurface(const SurfaceCIType& ci) const
{
	VkSurfaceKHR ret;
	vkCreateWin32SurfaceKHR(instance, &ci, nullptr, &ret);
	return ret;
}
