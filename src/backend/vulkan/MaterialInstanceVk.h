//
// Created by Frank on 2024/1/15.
//

#ifndef XD_RT_MATERIALINSTANCEVK_H
#define XD_RT_MATERIALINSTANCEVK_H
#include <memory>
#include <string>
#include <vector>
#include "VulkanTypes.h"
namespace xd {

class MaterialInstanceVk {
public:
	MaterialInstanceVk(std::shared_ptr<VulkanDevice> device,
					   std::shared_ptr<const MaterialTemplateVk> mtl_template);
	std::shared_ptr<VulkanDescriptorSet> queryDescriptorSet(const std::string& name) const;
	void updateDescriptorSets() const;
	void bindDescriptorSets(std::shared_ptr<VulkanCommandBuffer> cmdBuffer) const;

private:
	std::shared_ptr<VulkanDevice> device = nullptr;
	std::shared_ptr<const MaterialTemplateVk> mtlTemplate = nullptr;
	std::shared_ptr<VulkanDescriptorPool> pool = nullptr;
	std::vector<std::shared_ptr<VulkanDescriptorSet>> descriptorSets{};
};

}  // namespace xd

#endif	// XD_RT_MATERIALINSTANCEVK_H
