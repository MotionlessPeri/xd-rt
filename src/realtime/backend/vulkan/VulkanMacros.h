//
// Created by Frank on 2024/1/9.
//

#ifndef XD_RT_VULKANMACROS_H
#define XD_RT_VULKANMACROS_H
#include <iostream>
#include <stdexcept>
#include "VulkanPlatformSpecific.h"
#ifdef _DEBUG
#define CHECK_VK_ERROR(f)             \
	if ((f) != VK_SUCCESS) {          \
		throw std::runtime_error{""}; \
	}
#else
#define CHECK_VK_ERROR(f) (f)
#endif
#endif	// XD_RT_VULKANMACROS_H
