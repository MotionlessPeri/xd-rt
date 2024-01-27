//
// Created by Frank on 2024/1/24.
//

#ifndef XD_RT_MATERIALFACTORYVK_H
#define XD_RT_MATERIALFACTORYVK_H
#include <memory>
#include "VulkanTypes.h"
namespace xd {

class MaterialFactoryVk {
public:
	static void init(std::shared_ptr<VulkanDevice> device);
	static void terminate();
	static MaterialFactoryVk& get() { return *singleton; }
	std::shared_ptr<MaterialTemplateVk> createLambertian(
		std::shared_ptr<VulkanSubpass> subpass) const;

private:
	explicit MaterialFactoryVk(std::shared_ptr<VulkanDevice> device);

	static MaterialFactoryVk* singleton;
	std::shared_ptr<VulkanDevice> device;
};

}  // namespace xd

#endif	// XD_RT_MATERIALFACTORYVK_H
