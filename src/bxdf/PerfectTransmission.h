//
// Created by Frank on 2023/12/17.
//

#ifndef XD_RT_PERFECTTRANSMISSION_H
#define XD_RT_PERFECTTRANSMISSION_H
#include "BxDF.h"
namespace xd {
/**
 * Perfect transmission respect of eta = eta_t / eta_i.
 * The implementation assumes the normal is in the same hemisphere of wi.
 * If the light travels into less dense medium, the ray will be neglected. Notice that because of
 * this, this bxdf is not energy preserved(it is energy conserved though)
 */
class PerfectTransmission : public BxDF {
public:
	explicit PerfectTransmission(float eta);
	PerfectTransmission(float etaI, float etaT);
	ColorRGB getBxDF(const Vector3f& wi, const Vector3f& wo) const override;
	SampleBxDFResult sampleBxDF(const Vector2f& uSample, const Vector3f& wo) const override;
	SampleBxDFPdfResult sampleBxDFWithPdf(const Vector2f& uSample,
	                                      const Vector3f& wo) const override;
	Vector3f sampleDirection(const Vector2f& uSample, const Vector3f& wo) const override;
	SampleDirPdfResult sampleDirectionWithPdf(const Vector2f& uSample,
	                                          const Vector3f& wo) const override;
	float getPdf(const Vector3f& wi) const override;
	bool isDelta() const override;
	/**
	 * \brief check if total reflection is reached. If reached, isTotal is set to true; otherwise
	 * sin2ThetaT is provided
	 * \param cosThetaI the cosine of wo and n
	 * \return {isTotal, sin2ThetaT}
	 */
	static auto checkTotalReflection(float eta, float cosThetaI)
	{
		struct Ret {
			bool isTotalReflection;
			float sin2ThetaT;
		} ret;
		const float sin2ThetaI = 1 - cosThetaI * cosThetaI;
		ret.sin2ThetaT = sin2ThetaI / (eta * eta);
		ret.isTotalReflection = ret.sin2ThetaT > 1;
		return ret;
	}

protected:
	Vector3f refract(const Vector3f& wo, float cosThetaT) const;

	float eta;
};
}  // namespace xd
#endif	// XD_RT_PERFECTTRANSMISSION_H
