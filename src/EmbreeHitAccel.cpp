//
// Created by Frank on 2023/9/30.
//
#include "EmbreeGeomManager.h"
#include "EmbreeUtil.h"
#include "HitAccel.h"
#include "Primitive.h"
using namespace xd;
EmbreeAccel::EmbreeAccel(RTCDevice device, const std::vector<const Model*>& models) : device(device)
{
	scene = rtcNewScene(device);
	auto& embreeGeomManager = EmbreeGeomManager::get();
	for (const auto& model : models) {
		geomMap[rtcAttachGeometry(scene, embreeGeomManager.getOrCreateGeom(model))] = model;
	}
	rtcCommitScene(scene);
}
bool EmbreeAccel::hit(const Ray& ray, HitRecord& rec) const
{
	auto rtcRayHit = rayAndHitRecordToRTCRayHit(ray, rec);
	rtcIntersect1(scene, &rtcRayHit);
	if (rtcRayHit.hit.geomID == RTC_INVALID_GEOMETRY_ID)
		return false;
	const auto* model = geomMap.at(rtcRayHit.hit.geomID);
	rtcInterpolate0(rtcGetGeometry(scene, rtcRayHit.hit.geomID), rtcRayHit.hit.primID,
					rtcRayHit.hit.u, rtcRayHit.hit.v, RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
					(unsigned int)VertexAttributeSlot::UV, rec.uv.data(), 3);
	rtcInterpolate1(rtcGetGeometry(scene, rtcRayHit.hit.geomID), rtcRayHit.hit.primID,
					rtcRayHit.hit.u, rtcRayHit.hit.v, RTC_BUFFER_TYPE_VERTEX, 0, rec.tPoint.data(),
					rec.dpdu.data(), rec.dpdv.data(), 3);
	rec.n = {rtcRayHit.hit.Ng_x, rtcRayHit.hit.Ng_y, rtcRayHit.hit.Ng_z};
	rec.tHit = rtcRayHit.ray.tfar;
	const auto* prim = dynamic_cast<const Primitive*>(model);
	if (prim != nullptr) {
		rec.primitive = std::static_pointer_cast<const Primitive>(prim->shared_from_this());
	}
	return true;
}
