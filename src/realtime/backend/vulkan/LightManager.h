//
// Created by Frank on 2024/1/26.
//

#ifndef XD_RT_LIGHTMANAGER_H
#define XD_RT_LIGHTMANAGER_H
#include <glm/glm.hpp>
#include <memory>
#include <shared_mutex>
#include <vector>
#include "VulkanTypes.h"
namespace xd {
enum class LightType : uint32_t { POINT = 0 };
// we use std430 on PointLightInfo struct, which is tightly packed
struct PointLightInfo {
	PointLightInfo(const glm::vec3& _pos, const glm::vec3& _intensity)
		: pos(_pos), intensity(_intensity)
	{
	}

	glm::vec3 pos;
	float padding0;
	glm::vec3 intensity;
	float padding1;
};
struct LightIndex {
	uint32_t type;
	uint32_t index;
};
class LightManager {
public:
	explicit LightManager(std::shared_ptr<VulkanDevice> device);

	void addPointLight(const PointLightInfo& info);
	void bindLightInfos(std::shared_ptr<MaterialInstanceVk> mi);
	inline static constexpr uint32_t MAX_POINT_LIGHT_COUNT = 32;
	inline static constexpr uint32_t MAX_LIGHT_INDEX_COUNT = 1024;

private:
	void updateLightInfos();
	bool dirty = false;
	std::shared_mutex sharedMutex;
	std::shared_ptr<VulkanDevice> device = nullptr;
	std::vector<PointLightInfo> pointLightInfo{};
	std::vector<LightIndex> lightIndexes{};
	std::shared_ptr<VulkanBuffer> lightIndexBuffer = nullptr;
	std::shared_ptr<VulkanBuffer> pointLightBuffer = nullptr;
};

}  // namespace xd

#endif	// XD_RT_LIGHTMANAGER_H
