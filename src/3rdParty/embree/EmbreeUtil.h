//
// Created by Frank on 2023/9/30.
//

#ifndef XD_RT_EMBREEUTIL_H
#define XD_RT_EMBREEUTIL_H
#include "../../core/CoreTypes.h"
#include "../../core/HitRecord.h"
#include "../../core/Ray.h"
#include "embree4/rtcore.h"
namespace xd {
inline RTCRay rayToRTCRay(const Ray& ray)
{
	RTCRay rtcRay;
	rtcRay.org_x = ray.o.x();
	rtcRay.org_y = ray.o.y();
	rtcRay.org_z = ray.o.z();
	rtcRay.dir_x = ray.d.x();
	rtcRay.dir_y = ray.d.y();
	rtcRay.dir_z = ray.d.z();
	rtcRay.tnear = 0.f;
	rtcRay.tfar = std::numeric_limits<float>::infinity();
	rtcRay.mask = -1;
	rtcRay.flags = 0;
	return rtcRay;
}
inline RTCRayHit rayAndHitRecordToRTCRayHit(const Ray& ray, const HitRecord& rec)
{
	RTCRayHit rtcRayHit;
	rtcRayHit.ray.org_x = ray.o.x();
	rtcRayHit.ray.org_y = ray.o.y();
	rtcRayHit.ray.org_z = ray.o.z();
	rtcRayHit.ray.dir_x = ray.d.x();
	rtcRayHit.ray.dir_y = ray.d.y();
	rtcRayHit.ray.dir_z = ray.d.z();
	rtcRayHit.ray.tnear = 0.f;
	rtcRayHit.ray.tfar = rec.tHit;
	rtcRayHit.ray.mask = -1;
	rtcRayHit.ray.flags = 0;
	rtcRayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
	rtcRayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
	return rtcRayHit;
}
inline RTCRayHit rayToRTCRayHit(const Ray& ray)
{
	RTCRayHit rtcRayHit;
	rtcRayHit.ray = rayToRTCRay(ray);
	rtcRayHit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
	rtcRayHit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;
	return rtcRayHit;
}
}  // namespace xd
#endif	// XD_RT_EMBREEUTIL_H
