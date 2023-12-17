//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_COSINEHEMISPHERE_H
#define XD_RT_COSINEHEMISPHERE_H
#include "InverseMethodDistribution.h"
namespace xd {
class CosineHemisphere : public InverseMethodDistribution<3, 2> {
public:
	RetType sample(const UniformSampleType& uSample) override;
	RetType sampleWithPdf(const UniformSampleType& uSample, float& pdf) override;
	float getPdf(const Vector3f& sample) const override;
};

}  // namespace xd
#endif	// XD_RT_COSINEHEMISPHERE_H
