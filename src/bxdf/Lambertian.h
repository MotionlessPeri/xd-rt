//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_LAMBERTIAN_H
#define XD_RT_LAMBERTIAN_H
#include "BxDF.h"
#include "distribution/CosineHemisphere.h"
namespace xd {
class Lambertian : public BxDF {
public:
	explicit Lambertian(ColorRGB color);
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

protected:
	ColorRGB color;
	std::unique_ptr<CosineHemisphere> distrib;
};
}  // namespace xd
#endif	// XD_RT_LAMBERTIAN_H
