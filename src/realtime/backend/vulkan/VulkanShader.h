//
// Created by Frank on 2024/1/11.
//

#ifndef XD_RT_VULKANSHADER_H
#define XD_RT_VULKANSHADER_H
#include <string>
#include "VulkanDeviceObject.h"
#include "VulkanPlatformSpecific.h"
namespace xd {
enum class ShaderStage { VERTEX, FRAGMENT };
class VulkanShader : public VulkanDeviceObject<VkShaderModuleCreateInfo> {
public:
	friend class VulkanDevice;
	VulkanShader() = delete;
	VulkanShader(const VulkanShader& other) = delete;
	VulkanShader(VulkanShader&& other) noexcept = delete;
	VulkanShader& operator=(const VulkanShader& other) = delete;
	VulkanShader& operator=(VulkanShader&& other) noexcept = delete;
	~VulkanShader();
	VkPipelineShaderStageCreateInfo getShaderStageCi() const
	{
		VkPipelineShaderStageCreateInfo ci;
		ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ci.pNext = nullptr;
		ci.flags = 0;
		ci.stage = stage;
		ci.module = shaderModule;
		ci.pName = entryPointName.data();
		ci.pSpecializationInfo = nullptr;
		return ci;
	}
	VkShaderStageFlagBits getShaderStage() const { return stage; }

private:
	VulkanShader(std::shared_ptr<const VulkanDevice> device,
				 const VkShaderModuleCreateInfo& desc,
				 VkShaderModule shader_module,
				 VkShaderStageFlagBits stage,
				 std::string entry_point_name = "main");
	VkShaderModule shaderModule = VK_NULL_HANDLE;
	VkShaderStageFlagBits stage;
	std::string entryPointName = "main";
};

}  // namespace xd

#endif	// XD_RT_VULKANSHADER_H
