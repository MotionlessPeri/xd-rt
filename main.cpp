#include "tutorial/basicGraphic/VulkanGLFWApp.h"
using namespace xd;
int main()
{
	constexpr int WIDTH = 1000;
	constexpr int HEIGHT = 800;
	const char* TITLE = "glfw window test";
	VulkanGLFWApp app{WIDTH, HEIGHT, TITLE};
	app.run();
}
