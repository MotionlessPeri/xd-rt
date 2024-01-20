//
// Created by Frank on 2024/1/15.
//

#include "MaterialInstanceVk.h"
#include "MaterialTemplateVk.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanDevice.h"
using namespace xd;
MaterialInstanceVk::MaterialInstanceVk(std::shared_ptr<VulkanDevice> _device,
									   std::shared_ptr<const MaterialTemplateVk> mtl_template)
	: device(std::move(_device)), mtlTemplate(std::move(mtl_template))
{
	DescriptorPoolDesc desc;
	desc.poolSizes = mtlTemplate->getPoolSizes();
	desc.ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	desc.ci.poolSizeCount = desc.poolSizes.size();
	desc.ci.pPoolSizes = desc.poolSizes.data();
	pool = device->createDescriptorPool(desc);

	descriptorSets.resize(mtlTemplate->setLayouts.size());
	std::ranges::transform(
		mtlTemplate->setLayouts, descriptorSets.begin(),
		[&](const auto& setLayout) { return setLayout->createDescriptorSet(pool); });
}
