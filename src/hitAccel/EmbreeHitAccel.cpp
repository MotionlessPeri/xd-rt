//
// Created by Frank on 2023/9/30.
//
#include "3rdParty/embree/EmbreeGeomManager.h"
#include "3rdParty/embree/EmbreeUtil.h"
#include "HitAccel.h"
#include "Primitive.h"
using namespace xd;
EmbreeAccel::EmbreeAccel(RTCDevice device, const std::vector<const Primitive*>& primitives)
	: device(device)
{
	scene = rtcNewScene(device);
	auto& embreeGeomManager = EmbreeGeomManager::get();
	for (const auto& primitive : primitives) {
		auto rtcGeom = embreeGeomManager.getOrCreateGeom(primitive->getModel().get());
		auto rtcDefaultScene = embreeGeomManager.getDefaultScene(rtcGeom);
		auto rtcInstance = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_INSTANCE);
		rtcSetGeometryInstancedScene(rtcInstance, rtcDefaultScene);
		rtcSetGeometryTimeStepCount(rtcInstance, 1);
		rtcSetGeometryTransform(rtcInstance, 0, RTC_FORMAT_FLOAT4X4_COLUMN_MAJOR,
								primitive->getLocalToWorld().data());
		rtcCommitGeometry(rtcInstance);
		auto rtcInstanceSceneId = rtcAttachGeometry(scene, rtcInstance);
		instanceIdToPrimitiveMap[rtcInstanceSceneId] = primitive;
		instanceIdToGeomMap[rtcInstanceSceneId] = rtcGeom;
	}
	rtcCommitScene(scene);
	// RTCBounds bounds;
	// rtcGetSceneBounds(scene, &bounds);
}
bool EmbreeAccel::hit(const Ray& ray, HitRecord& rec) const
{
	auto rtcRayHit = rayAndHitRecordToRTCRayHit(ray, rec);
	rtcIntersect1(scene, &rtcRayHit);
	if (rtcRayHit.hit.geomID == RTC_INVALID_GEOMETRY_ID)
		return false;
	const auto* primitive = instanceIdToPrimitiveMap.at(rtcRayHit.hit.instID[0]);
	const auto rtcGeom = instanceIdToGeomMap.at(rtcRayHit.hit.instID[0]);
	rtcInterpolate0(rtcGeom, rtcRayHit.hit.primID, rtcRayHit.hit.u, rtcRayHit.hit.v,
					RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, (unsigned int)VertexAttributeSlot::UV,
					rec.uv.data(), 2);
	rtcInterpolate1(rtcGeom, rtcRayHit.hit.primID, rtcRayHit.hit.u, rtcRayHit.hit.v,
					RTC_BUFFER_TYPE_VERTEX, 0, rec.p.data(), rec.dpdu.data(), rec.dpdv.data(), 3);

	rtcInterpolate0(rtcGeom, rtcRayHit.hit.primID, rtcRayHit.hit.u, rtcRayHit.hit.v,
					RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, (unsigned int)VertexAttributeSlot::NORMAL,
					rec.n.data(), 3);
	// rec.n = {rtcRayHit.hit.Ng_x, rtcRayHit.hit.Ng_y, rtcRayHit.hit.Ng_z};
	rec.n = (primitive->getLocalToWorld().linear().inverse().transpose() * rec.n).normalized();
	rec.tHit = rtcRayHit.ray.tfar;
	rec.primitive = std::static_pointer_cast<const Primitive>(primitive->shared_from_this());
	return true;
}
bool EmbreeAccel::hitAnything(const Ray& ray, HitRecord& rec) const
{
	auto rtcRay = rayToRTCRay(ray);
	rtcOccluded1(scene, &rtcRay);
	return rtcRay.tfar < 0;
}
