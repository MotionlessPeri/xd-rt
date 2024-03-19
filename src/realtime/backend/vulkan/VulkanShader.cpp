//
// Created by Frank on 2024/1/11.
//

#include "VulkanShader.h"

#include "VulkanDevice.h"
using namespace xd;

VulkanShader::~VulkanShader()
{
	device->destroyShader(shaderModule);
}

VulkanShader::VulkanShader(std::shared_ptr<const VulkanDevice> device,
						   const VkShaderModuleCreateInfo& desc,
						   VkShaderModule shader_module,
						   VkShaderStageFlagBits stage,
						   std::string entry_point_name)
	: VulkanDeviceObject(std::move(device), desc),
	  shaderModule(shader_module),
	  stage(stage),
	  entryPointName(std::move(entry_point_name))
{
}