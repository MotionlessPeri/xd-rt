//
// Created by Frank on 2024/1/11.
//

#ifndef XD_RT_GLFWGLOBAL_H
#define XD_RT_GLFWGLOBAL_H
#include <mutex>
#include "GLFWInclude.h"

namespace xd {
static std::once_flag glfwInitFlag;
class GLFWGlobal {
public:
	static void init()
	{
		std::call_once(glfwInitFlag, [&]() { glfwInit(); });
	}
};
};		// namespace xd
#endif	// XD_RT_GLFWGLOBAL_H
