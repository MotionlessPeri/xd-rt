//
// Created by Frank on 2024/1/11.
//
#include "app/vulkan/GLFWGlobal.h"
#include "app/vulkan/VulkanGLFWApp.h"
#include "gtest/gtest.h"
using namespace xd;
TEST(VulkanTestSuite, GLFWWindowTest)
{
	GLFWGlobal::init();
	constexpr int WIDTH = 1000;
	constexpr int HEIGHT = 800;
	const char* TITLE = "glfw window test";
	VulkanGLFWApp app{WIDTH, HEIGHT, TITLE};
	app.run();
}