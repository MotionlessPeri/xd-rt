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
// Note: we make an appointment that when HitRecord is filled by shape's hit method, the normal
// orientation is always pointing to "outside" of the model, if possible
struct HitRecord {
public:
	explicit HitRecord(float t_hit) : tHit(t_hit) {}
	HitRecord(float t_hit,
			  Vector3f p,
			  Vector2f uv,
			  const Vector3f& n,
			  std::shared_ptr<const Primitive> primitive)
		: tHit(t_hit), p(std::move(p)), uv(std::move(uv)), n(n), primitive(std::move(primitive))
	{
		std::tie(dpdu, dpdv) = coordSystem(n);
	}

	HitRecord(float t_hit,
			  Vector3f p,
			  Vector2f uv,
			  Vector3f n,
			  Vector3f dpdu,
			  Vector3f dpdv,
			  std::shared_ptr<const Primitive> primitive)
		: tHit(t_hit),
		  p(std::move(p)),
		  uv(std::move(uv)),
		  n(std::move(n)),
		  dpdu(std::move(dpdu)),
		  dpdv(std::move(dpdv)),
		  primitive(std::move(primitive))
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
