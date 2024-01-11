//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_VULKANGLFWWINDOW_H
#define XD_RT_VULKANGLFWWINDOW_H
#include "GLFWInclude.h"
#include "Types.h"
#include "backend/vulkan/VulkanGlobal.h"
namespace xd {

class VulkanGLFWApp {
public:
	VulkanGLFWApp(int width, int height, const char* title);
	void run();

private:
	GLFWwindow* window = nullptr;
};

}  // namespace xd

#endif	// XD_RT_VULKANGLFWWINDOW_H
