//
// Created by Frank on 2023/8/19.
//

#ifndef XD_RT_HITRECORD_H
#define XD_RT_HITRECORD_H

#include "CoreTypes.h"
#include "MathType.h"
#include "MathUtil.h"

namespace xd {
struct HitRecord {
public:
	explicit HitRecord(float t_hit) : tHit(t_hit) {}
	HitRecord(float t_hit,
			  const Vector3f& p,
			  const Vector2f& uv,
			  const Vector3f& n,
			  const std::shared_ptr<const Primitive>& primitive)
		: tHit(t_hit), p(p), uv(uv), n(n), primitive(primitive)
	{
		std::tie(dpdu, dpdv) = coordSystem(n);
		buildFrames();
	}

	HitRecord(float t_hit,
			  const Vector3f& p,
			  const Vector2f& uv,
			  const Vector3f& n,
			  const Vector3f& dpdu,
			  const Vector3f& dpdv,
			  const std::shared_ptr<const Primitive>& primitive)
		: tHit(t_hit), p(p), uv(uv), n(n), dpdu(dpdu), dpdv(dpdv), primitive(primitive)
	{
		buildFrames();
	}

	HitRecord() = default;
	HitRecord(const HitRecord& other) = default;
	HitRecord(HitRecord&& other) noexcept = default;
	HitRecord& operator=(const HitRecord& other) = default;
	HitRecord& operator=(HitRecord&& other) noexcept = default;
	void buildFrames()
	{
		const Vector3f tx = dpdu.normalized();
		const Vector3f ty = n.cross(dpdu.normalized());
		localToModel << tx, ty, n;
		modelToLocal = localToModel.inverse();
	}
	float tHit = FLT_MAX;
	Vector3f p;
	// param representation
	Vector2f uv;
	// local frame
	Vector3f n;
	Vector3f dpdu;
	Vector3f dpdv;
	std::shared_ptr<const Primitive> primitive = nullptr;
	Matrix3f localToModel;
	Matrix3f modelToLocal;

protected:
};
}  // namespace xd
#endif	// XD_RT_HITRECORD_H
