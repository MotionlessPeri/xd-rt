//
// Created by Frank on 2023/9/30.
//
#include "EmbreeHitAccel.h"
#include "3rdParty/embree/EmbreeGeomManager.h"
#include "3rdParty/embree/EmbreeUtil.h"
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
								primitive->getModelToWorld().data());
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
	const auto rtcInstance = rtcGetGeometry(scene, rtcRayHit.hit.instID[0]);
	const auto* primitive = instanceIdToPrimitiveMap.at(rtcRayHit.hit.instID[0]);
	const auto rtcGeom = instanceIdToGeomMap.at(rtcRayHit.hit.instID[0]);
	rtcInterpolate0(rtcGeom, rtcRayHit.hit.primID, rtcRayHit.hit.u, rtcRayHit.hit.v,
					RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, (unsigned int)VertexAttributeSlot::UV,
					rec.geom.uv.data(), 2);
	rtcInterpolate1(rtcGeom, rtcRayHit.hit.primID, rtcRayHit.hit.u, rtcRayHit.hit.v,
					RTC_BUFFER_TYPE_VERTEX, 0, rec.geom.p.data(), rec.geom.derivatives.dpdu.data(),
					rec.geom.derivatives.dpdv.data(), 3);

	rtcInterpolate0(rtcGeom, rtcRayHit.hit.primID, rtcRayHit.hit.u, rtcRayHit.hit.v,
					RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE, (unsigned int)VertexAttributeSlot::NORMAL,
					rec.geom.derivatives.n.data(), 3);
	float rawModelToWorld[16];
	rtcGetGeometryTransformFromScene(scene, rtcRayHit.hit.instID[0], 0,
									 RTC_FORMAT_FLOAT4X4_COLUMN_MAJOR, rawModelToWorld);
	const Transform modelToWorld{Matrix4f{rawModelToWorld}};
	rec.applyTransform(FrameCategory::WORLD, modelToWorld);
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
