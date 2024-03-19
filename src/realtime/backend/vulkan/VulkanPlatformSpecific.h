//
// Created by Frank on 2024/1/10.
//

#ifndef XD_RT_VULKANPLATFORMSPECIFIC_H
#define XD_RT_VULKANPLATFORMSPECIFIC_H

#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX
#endif

#include "vulkan/vulkan.h"

namespace xd {
#ifdef WIN32
using SurfaceCIType = VkWin32SurfaceCreateInfoKHR;
#endif

#ifdef WIN32
using NativeWindowType = HWND;
#endif
}  // namespace xd
#endif	// XD_RT_VULKANPLATFORMSPECIFIC_H
