//
// Created by Frank on 2024/1/15.
//

#ifndef XD_RT_MATERIALTEMPLATEVK_H
#define XD_RT_MATERIALTEMPLATEVK_H
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "VulkanGraphicsPipeline.h"
#include "VulkanPipelineLayout.h"
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
namespace xd {

class MaterialTemplateVk : public std::enable_shared_from_this<MaterialTemplateVk> {
public:
	friend class MaterialInstanceVk;

	MaterialTemplateVk(std::shared_ptr<VulkanDevice> device,
					   std::vector<std::shared_ptr<VulkanShader>> shaders,
					   std::unordered_map<std::string, uint32_t> set_name_to_index,
					   const std::shared_ptr<VulkanPipelineBase>& pipeline);

	std::shared_ptr<MaterialInstanceVk> createInstance() const;
	std::vector<VkDescriptorPoolSize> getPoolSizes() const;
	uint32_t querySetIndex(const std::string& name) const;
	const std::vector<std::shared_ptr<VulkanDescriptorSetLayout>>& getLayouts() const
	{
		return pipeline->getLayout()->getDesc().setLayouts;
	}
	void bindPipeline(std::shared_ptr<VulkanCommandBuffer>) const;

private:
	std::shared_ptr<VulkanDevice> device = nullptr;
	std::vector<std::shared_ptr<VulkanShader>> shaders{};
	std::unordered_map<std::string, uint32_t> setNameToIndex;
	// Note: a material template may be mapped to several pipelines. The variant of materials can be
	// explained to variant to pipeline mapping. But for convenience we use a single pipeline
	// instance for now
	std::shared_ptr<VulkanPipelineBase> pipeline = nullptr;
};

}  // namespace xd

#endif	// XD_RT_MATERIALTEMPLATEVK_H
