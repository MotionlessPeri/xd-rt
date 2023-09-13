//
// Created by Frank on 2023/8/19.
//

#ifndef XD_RT_HITRECORD_H
#define XD_RT_HITRECORD_H

#include "CoreTypes.h"
#include "MathType.h"

namespace xd {
struct HitRecord {
	float tHit = FLT_MAX;
	// param representation
	Vector2f uv;
	// local frame
	Vector3f dpdu;
	Vector3f dpdv;
	Vector3f n;
	std::shared_ptr<Primitive> primitive = nullptr;
	Matrix3f getLocalToWorld() const
	{
		Matrix3f localToWorld;
		localToWorld << dpdu.normalized(), dpdv.normalized(), n;
		return localToWorld;
	}
	uint32_t debug = 0;
};
}  // namespace xd
#endif	// XD_RT_HITRECORD_H
