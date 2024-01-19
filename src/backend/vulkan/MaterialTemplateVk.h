//
// Created by Frank on 2024/1/15.
//

#ifndef XD_RT_MATERIALTEMPLATEVK_H
#define XD_RT_MATERIALTEMPLATEVK_H
#include <memory>
#include <vector>
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {

class MaterialTemplateVk : public std::enable_shared_from_this<MaterialTemplateVk> {
public:
	friend class MaterialInstanceVk;

	MaterialTemplateVk(std::shared_ptr<VulkanDevice> device,
					   std::vector<std::shared_ptr<VulkanShader>> shaders,
					   std::vector<std::shared_ptr<VulkanDescriptorSetLayout>> set_layouts,
					   std::shared_ptr<VulkanGraphicsPipeline> pipeline);

	std::shared_ptr<MaterialInstanceVk> createInstance() const;
	std::vector<VkDescriptorPoolSize> getPoolSizes() const;

private:
	std::shared_ptr<VulkanDevice> device = nullptr;
	std::vector<std::shared_ptr<VulkanShader>> shaders{};
	std::vector<std::shared_ptr<VulkanDescriptorSetLayout>> setLayouts{};
	std::shared_ptr<VulkanGraphicsPipeline> pipeline = nullptr;
};

}  // namespace xd

#endif	// XD_RT_MATERIALTEMPLATEVK_H
