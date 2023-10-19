//
// Created by Frank on 2023/9/30.
//

#ifndef XD_RT_EMBREEGEOMMANAGER_H
#define XD_RT_EMBREEGEOMMANAGER_H
#include <memory>
#include <unordered_map>
#include "CoreTypes.h"
#include "embree4/rtcore.h"
namespace xd {
enum class VertexAttributeSlot : unsigned int { UV = 0, NORMAL = 1, TANGENT = 2 };
class EmbreeGeomManager {
public:
	static EmbreeGeomManager& get()
	{
		static EmbreeGeomManager* singleton = new EmbreeGeomManager;
		return *singleton;
	}
	RTCGeometry getOrCreateGeom(const Model* model);
	RTCScene getDefaultScene(const RTCGeometry& rtcGeom);
	void releaseGeom(const Model* model);

protected:
	EmbreeGeomManager();
	~EmbreeGeomManager() = default;
	RTCDevice device;
	std::unordered_map<const Model*, RTCGeometry> geoms;
	std::unordered_map<RTCGeometry, RTCScene> defaultScenes;
};
}  // namespace xd
#endif	// XD_RT_EMBREEGEOMMANAGER_H
