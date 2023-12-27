//
// Created by Frank on 2023/8/19.
//

#ifndef XD_RT_HITRECORD_H
#define XD_RT_HITRECORD_H

#include <optional>

#include "CoreTypes.h"
#include "Enums.h"
#include "Material.h"
#include "MathTypes.h"
#include "MathUtil.h"
#include "Ray.h"
namespace xd {
struct ShadingDerivatives {
	ShadingDerivatives() = default;
	ShadingDerivatives(const ShadingDerivatives& other) = default;
	ShadingDerivatives(ShadingDerivatives&& other) noexcept = default;
	ShadingDerivatives& operator=(const ShadingDerivatives& other) = default;
	ShadingDerivatives& operator=(ShadingDerivatives&& other) noexcept = default;
	~ShadingDerivatives() = default;
	ShadingDerivatives(Vector3f n, Vector3f dpdu, Vector3f dpdv, Vector3f dndu, Vector3f dndv);

	/**
	 * \brief construct a new ShadingGeometry by twisting other.n to shadingNormal
	 * \param other another ShadingGeometry instance
	 * \param shadingNormal the normal needs to be replaced
	 */
	ShadingDerivatives(const ShadingDerivatives& other, Vector3f shadingNormal);
	void applyTransform(const Transform& transform);
	Vector3f n;
	Vector3f dpdu, dpdv;
	Vector3f dndu{1, 0, 0}, dndv{0, 1, 0};
};

struct LocalGeomParams {
public:
	LocalGeomParams() = default;
	LocalGeomParams(const LocalGeomParams& other) = default;
	LocalGeomParams(LocalGeomParams&& other) noexcept = default;
	LocalGeomParams& operator=(const LocalGeomParams& other) = default;
	LocalGeomParams& operator=(LocalGeomParams&& other) noexcept = default;
	~LocalGeomParams() = default;
	LocalGeomParams(FrameCategory frame,
					Vector3f p,
					Vector2f uv,
					Vector3f n,
					Vector3f dpdu,
					Vector3f dpdv,
					Vector3f dndu,
					Vector3f dndv);
	LocalGeomParams(FrameCategory frame, Vector3f p, Vector2f uv, ShadingDerivatives geom);
	explicit operator ShadingDerivatives() const;

	Transform getCurrentFrame() const;

	void setShadingGeometry(ShadingDerivatives geom);
	void applyTransform(FrameCategory frame,
						const Transform& transform,
						Vector3f* pError = nullptr);
	FrameCategory frame = FrameCategory::UNKNOWN;
	Vector3f p;
	Vector2f uv;
	ShadingDerivatives derivatives;
};

// Note: we make an appointment that when HitRecord is filled by shape's hit method, the normal
// orientation is always pointing to "outside" of the model, if possible
struct HitRecord {
public:
	explicit HitRecord(float t_hit);
	HitRecord(float t_hit,
			  Vector3f p_error,
			  std::shared_ptr<const Primitive> primitive,
			  FrameCategory frame,
			  Vector3f p,
			  Vector2f uv,
			  Vector3f n,
			  Vector3f dpdu,
			  Vector3f dpdv,
			  Vector3f dndu,
			  Vector3f dndv);
	HitRecord() = default;
	HitRecord(const HitRecord& other) = default;
	HitRecord(HitRecord&& other) noexcept = default;
	HitRecord& operator=(const HitRecord& other) = default;
	HitRecord& operator=(HitRecord&& other) noexcept = default;

	void applyTransform(FrameCategory frame, const Transform& transform);
	/**
	 * \brief spawn a new ray according to direction w, pError is considered to offset ray origin
	 * \param w the new ray direction in world frame
	 * \return the new ray
	 */
	Ray spawnRay(const Vector3f& w) const;
	std::shared_ptr<PhysicalPlausibleMaterial> getPhysicalPlausibleMaterial() const;
	/******** Material Sampling Methods Begin ********/
	const ShadingDerivatives& getShadingGeometry() const;
	LocalGeomParams getShadingGeomParams() const;
	ColorRGB getBxDF(const Vector3f& wo, const Vector3f& wi) const;
	SampleBxDFResult sampleBxDF(const Vector2f& uSample, const Vector3f& wo) const;
	SampleBxDFPdfResult sampleBxDFPdf(const Vector2f& uSample, const Vector3f& wo) const;
	Vector3f sampleMaterialWi(const Vector2f& uSample, const Vector3f& wo) const;
	SampleDirPdfResult sampleMaterialWiPdf(const Vector2f& uSample, const Vector3f& wo) const;
	float getMaterialPdf(const Vector3f& wo) const;
	/******** Material Sampling Methods End   ********/
	float tHit = FLT_MAX;
	std::shared_ptr<const Primitive> primitive = nullptr;
	LocalGeomParams geom;
	Vector3f pError{0, 0, 0};  // tracking geom.p's error accumulation

protected:
	mutable std::optional<ShadingDerivatives> shadingDerivatives = std::nullopt;
};
}  // namespace xd
#endif	// XD_RT_HITRECORD_H
