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
	SampleBxDFResult sampleBxDF(const Vector2f& uSample, const Vector3f& wo) const override;
	SampleBxDFPdfResult sampleBxDFWithPdf(const Vector2f& uSample,
										  const Vector3f& wo) const override;
	Vector3f sampleDirection(const Vector2f& uSample, const Vector3f& wo) const override;
	SampleDirPdfResult sampleDirectionWithPdf(const Vector2f& uSample,
											  const Vector3f& wo) const override;
	float getPdf(const Vector3f& wo) const override;
	bool isDelta() const override;

protected:
	ColorRGB color;
	std::unique_ptr<CosineHemisphere> distrib;
};
}  // namespace xd
#endif	// XD_RT_LAMBERTIAN_H
