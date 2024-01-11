//
// Created by Frank on 2024/1/11.
//

#ifndef XD_RT_GLFWINCLUDE_H
#define XD_RT_GLFWINCLUDE_H
#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif
#endif	// XD_RT_GLFWINCLUDE_H
