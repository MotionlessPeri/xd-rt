//
// Created by Frank on 2024/1/30.
//

#ifndef XD_RT_SHADERBASE_H
#define XD_RT_SHADERBASE_H
#include <memory>
#include "realtime/backend/vulkan/VulkanTypes.h"
namespace xd {
class ShaderBase {
public:
	explicit ShaderBase(std::shared_ptr<VulkanShader> shader);

protected:
	std::shared_ptr<VulkanShader> shader = nullptr;
};
};		// namespace xd
#endif	// XD_RT_SHADERBASE_H
