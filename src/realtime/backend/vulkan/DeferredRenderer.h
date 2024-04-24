//
// Created by Frank on 2024/4/24.
//

#ifndef XD_RAYTRACING_DEFERREDRENDERER_H
#define XD_RAYTRACING_DEFERREDRENDERER_H
#include "FrameGraph.h"
#include "MaterialFactoryVk.h"
#include "VulkanTypes.h"
namespace xd {
class DeferredRenderer {
public:
	DeferredRenderer(std::shared_ptr<VulkanDevice> pDevice);

protected:
	std::shared_ptr<VulkanDevice> device;
	std::shared_ptr<FrameGraph> fg;
	FGBuilder builder;
	struct Material {
		std::shared_ptr<MaterialTemplateVk> mtlTemplate = nullptr;
		std::shared_ptr<MaterialInstanceVk> mtlInstance = nullptr;
	} gBufferPassMtl, lightPassMtl;
};
}  // namespace xd
#endif	// XD_RAYTRACING_DEFERREDRENDERER_H
