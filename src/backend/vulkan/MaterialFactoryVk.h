//
// Created by Frank on 2024/1/24.
//

#ifndef XD_RT_MATERIALFACTORYVK_H
#define XD_RT_MATERIALFACTORYVK_H
#include <memory>
#include <string>
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {

class MaterialFactoryVk {
public:
	static void init(std::shared_ptr<VulkanDevice> device);
	static void terminate();
	static MaterialFactoryVk& get() { return *singleton; }
	std::shared_ptr<MaterialTemplateVk> createLambertianMaterial(
		std::shared_ptr<VulkanSubpass> subpass) const;
	std::shared_ptr<MaterialTemplateVk> createTonemappingMaterial() const;

private:
	explicit MaterialFactoryVk(std::shared_ptr<VulkanDevice> device);
	std::shared_ptr<VulkanShader> createShader(const std::string& path,
											   VkShaderStageFlagBits stage) const;
	static MaterialFactoryVk* singleton;
	std::shared_ptr<VulkanDevice> device;
};

}  // namespace xd

#endif	// XD_RT_MATERIALFACTORYVK_H
