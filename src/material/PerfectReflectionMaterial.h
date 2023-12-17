//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_PERFECTREFLECTIONMATERIAL_H
#define XD_RT_PERFECTREFLECTIONMATERIAL_H
#include "Material.h"
#include "bxdf/PerfectReflection.h"
namespace xd {

class PerfectReflectionMaterial : public Material {
public:
	PerfectReflectionMaterial();
	ColorRGB getBxDF(const HitRecord& primRec,
					 const Vector3f& wo,
					 const Vector3f& wi) const override;
	ColorRGB sampleBxDF(const Vector2f& uSample,
						const HitRecord& primRec,
						const Vector3f& wo,
						Vector3f& wi) override;
	ColorRGB sampleBxDFWithPdf(const Vector2f& uSample,
							   const HitRecord& primRec,
							   const Vector3f& wo,
							   Vector3f& wi,
							   float& pdf) override;
	Vector3f sampleDirection(const Vector2f& uSample,
							 const HitRecord& primRec,
							 const Vector3f& wo) const override;
	Vector3f sampleDirectionWithPdf(const Vector2f& uSample,
									const HitRecord& primRec,
									const Vector3f& wo,
									float& pdf) const override;
	float getPdf(const HitRecord& primRec, const Vector3f& wo) const override;
	bool isDelta() const override { return true; }

protected:
	std::unique_ptr<PerfectReflection> brdf;
};
}  // namespace xd
#endif	// XD_RT_PERFECTREFLECTIONMATERIAL_H
