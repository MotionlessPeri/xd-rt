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
	SampleBxDFResult sampleBxDF(const Vector2f& uSample, const Vector3f& wo) const override;
	SampleBxDFPdfResult sampleBxDFWithPdf(const Vector2f& uSample,
	                                      const Vector3f& wo) const override;
	Vector3f sampleDirection(const Vector2f& uSample, const Vector3f& wo) const override;
	SampleDirPdfResult sampleDirectionWithPdf(const Vector2f& uSample,
	                                          const Vector3f& wo) const override;
	float getPdf(const Vector3f& wo) const override;
	bool isDelta() const override;
};

}  // namespace xd
#endif	// XD_RT_PERFECTREFLECTION_H
