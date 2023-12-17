//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_PERFECTREFLECTION_H
#define XD_RT_PERFECTREFLECTION_H
#include "BxDF.h"
namespace xd {

class PerfectReflection : public BxDF {
public:
	~PerfectReflection() = default;
	ColorRGB getBxDF(const Vector3f& wi, const Vector3f& wo) const override;
	ColorRGB sampleBxDF(const Vector2f& uSample, const Vector3f& wo, Vector3f& wi) override;
	ColorRGB sampleBxDFWithPdf(const Vector2f& uSample,
							   const Vector3f& wo,
							   Vector3f& wi,
							   float& pdf) override;
	Vector3f sampleDirection(const Vector2f& uSample, const Vector3f& wo) const override;
	Vector3f sampleDirectionWithPdf(const Vector2f& uSample,
									const Vector3f& wo,
									float& pdf) override;
	float getPdf(const Vector3f& wo) const override;
	bool isDelta() const override;
};

}  // namespace xd
#endif	// XD_RT_PERFECTREFLECTION_H
