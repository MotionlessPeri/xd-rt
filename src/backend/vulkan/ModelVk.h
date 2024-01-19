//
// Created by Frank on 2024/1/13.
//

#ifndef XD_RT_MODELVK_H
#define XD_RT_MODELVK_H
#include "VulkanPlatformSpecific.h"
namespace xd {

class ModelVk {
public:
	friend class ModelFactoryVk;
private:
	VkBuffer position{VK_NULL_HANDLE}, uv{VK_NULL_HANDLE}, normal{VK_NULL_HANDLE},
		tangent{VK_NULL_HANDLE};
	VkBuffer indices{VK_NULL_HANDLE};
};

}  // namespace xd

#endif	// XD_RT_MODELVK_H
