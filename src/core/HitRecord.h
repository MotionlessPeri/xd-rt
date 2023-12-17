//
// Created by Frank on 2023/8/19.
//

#ifndef XD_RT_HITRECORD_H
#define XD_RT_HITRECORD_H

#include "CoreTypes.h"
#include "Enums.h"
#include "MathTypes.h"
#include "MathUtil.h"
#include "Ray.h"
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
	}

	HitRecord() = default;
	HitRecord(const HitRecord& other) = default;
	HitRecord(HitRecord&& other) noexcept = default;
	HitRecord& operator=(const HitRecord& other) = default;
	HitRecord& operator=(HitRecord&& other) noexcept = default;

	Matrix3f getCurrentFrame() const
	{
		Matrix3f res;
		const auto& z = n;
		const auto x = dpdu.normalized();
		const auto y = coordSystem(n, x);
		return buildFrameMatrix(x, y, z);
	}
	/**
	 * \brief spawn a new ray according to direction w, pError is considered to offset ray origin
	 * \param w the new ray direction in world frame
	 * \return the new ray
	 */
	Ray spawnRay(const Vector3f& w) const { return {offsetRayOrigin(p, pError, n, w), w}; }
	FrameCategory frame = FrameCategory::MODEL;
	float tHit = FLT_MAX;
	Vector3f p;
	Vector3f pError{0, 0, 0};
	// param representation
	Vector2f uv;
	// local frame
	Vector3f n;
	Vector3f dpdu;
	Vector3f dpdv;
	std::shared_ptr<const Primitive> primitive = nullptr;

protected:
};
}  // namespace xd
#endif	// XD_RT_HITRECORD_H
