//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_EMBREEHITACCEL_H
#define XD_RT_EMBREEHITACCEL_H
#include "HitAccel.h"
#include "embree4/rtcore.h"
namespace xd {
/**
 * @brief Hit acceleration structure using Intel's Embree library
 */
class EmbreeAccel : public HitAccel {
public:
	EmbreeAccel(RTCDevice device, const std::vector<const Primitive*>& primitives);
	bool hit(const Ray& ray, HitRecord& rec) const override;
	bool hitAnything(const Ray& ray, HitRecord& rec) const override;

protected:
	RTCDevice device;
	RTCScene scene;
	std::unordered_map<unsigned int, const Primitive*> instanceIdToPrimitiveMap;
	std::unordered_map<unsigned int, RTCGeometry> instanceIdToGeomMap;
};
}  // namespace xd
#endif	// XD_RT_EMBREEHITACCEL_H
