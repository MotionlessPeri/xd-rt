//
// Created by Frank on 2024/1/13.
//

#ifndef XD_RT_MODELFACTORY_H
#define XD_RT_MODELFACTORY_H
#include <array>
#include <memory>
#include <ranges>

#include "VulkanDescs.h"
#include "VulkanPlatformSpecific.h"
#include "VulkanTypes.h"
#include "model/ModelTypes.h"
namespace xd {
enum class UVType {
	Zero  // Set uv to zero
};
struct BuildTriangleMeshOptions {
	UVType fallbackUVType = UVType::Zero;
};
class ModelFactoryVk {
public:
	static void init(std::shared_ptr<VulkanDevice> device);
	static void terminate();
	static ModelFactoryVk& get() { return *singleton; }
	VertexInputStateDesc getVertexInputState() const
	{
		VertexInputStateDesc ret;
		ret.bindingDescs.resize(4);
		// we need pos, uv, normal and tangent for shading
		uint32_t strides[4] = {3 * sizeof(float), 2 * sizeof(float), 3 * sizeof(float),
							   3 * sizeof(float)};
		for (const auto i : std::views::iota(0u, ret.bindingDescs.size())) {
			auto& bDesp = ret.bindingDescs[i];
			bDesp.binding = i;
			bDesp.stride = strides[i];
			bDesp.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		}

		ret.attrDescs.resize(4);
		VkFormat vFormats[4] = {VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32_SFLOAT,
								VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT};
		for (const auto i : std::views::iota(0u, ret.attrDescs.size())) {
			auto& aDesp = ret.attrDescs[i];
			aDesp.binding = i;
			aDesp.location = i;
			aDesp.format = vFormats[i];
			aDesp.offset = 0;
		}
		ret.ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		ret.ci.pNext = nullptr;
		ret.ci.flags = 0;
		ret.ci.vertexBindingDescriptionCount = ret.bindingDescs.size();
		ret.ci.pVertexBindingDescriptions = ret.bindingDescs.data();
		ret.ci.vertexAttributeDescriptionCount = ret.attrDescs.size();
		ret.ci.pVertexAttributeDescriptions = ret.attrDescs.data();
		return ret;
	}
	ModelFactoryVk(const ModelFactoryVk& other) = delete;
	ModelFactoryVk(ModelFactoryVk&& other) noexcept = delete;
	ModelFactoryVk& operator=(const ModelFactoryVk& other) = delete;
	ModelFactoryVk& operator=(ModelFactoryVk&& other) noexcept = delete;
	std::shared_ptr<TriangleMeshVk> buildTriangleMesh(std::shared_ptr<TriangleMesh> mesh,
													  BuildTriangleMeshOptions options = {}) const;

private:
	ModelFactoryVk(std::shared_ptr<VulkanDevice> device);
	std::shared_ptr<VulkanDevice> device;
	static ModelFactoryVk* singleton;
};

}  // namespace xd

#endif	// XD_RT_MODELFACTORY_H
